//  Copyright (C) 2005-2011 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under
//  the terms of  the GNU General Public License as  published by the Free
//  Software  Foundation;  either version 2  of the  License,  or (at your
//  option) any later version.
//
//  This program  is distributed in the  hope that it will  be useful, but
//  WITHOUT   ANY  WARRANTY;   without  even   the  implied   warranty  of
//  MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
//  General Public License for more details.

#include <cmath>

#include <ogl-opengl.hpp>

#include <etc-math.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "math3d.h"
#include "scm-viewer.hpp"

//------------------------------------------------------------------------------

scm_viewer::scm_viewer(const std::string& exe,
                       const std::string& tag) : app::prog(exe, tag),
    cache  (0),
    model  (0),
    label  (0),
    timer  (0),
    timer_d(0),
    timer_e(0),
    height (0),
    radius (6),
    debug_cache(false),
    debug_label(false),
    debug_path (false),
    debug_wire (false),
    debug_bound(false)
{
    // Cache the label font file.

    font_ptr = ::data->load(::conf->get_s("sans_font"), &font_len);

    TIFFSetWarningHandler(0);
    gui_init();
}

scm_viewer::~scm_viewer()
{
    gui_free();
    unload();

    ::data->free(::conf->get_s("sans_font"));
}

//------------------------------------------------------------------------------

scm_frame::scm_frame(scm_cache *cache, app::node node) : mono(true)
{
    for (app::node n = node.find("scm"); n; n = node.next(n, "scm"))
    {
        image I;

        I.file = cache->add_file(n.get_s("file"),
                                 n.get_f("r0", 1.0),
                                 n.get_f("r1", 1.0));

        I.shader  = (n.get_s("shader") == "vert") ? 0 : 1;
        I.channel = (n.get_i("channel"));

        if (I.channel) mono = false;

        images.push_back(I);
    }
}

void scm_viewer::load(const std::string& name)
{
    // If the named file exists and contains an XML panorama definition...

    app::file file(name);

    if (app::node root = file.get_root().find("sphere"))
    {
        // Clear out the existing data.

        unload();

        // Parse the panorama configuration

        int n  = root.get_i("mesh",  16);
        int s  = root.get_i("size", 512);

        height = root.get_f("height", 0.0);
        radius = root.get_f("radius", 6.0);

        // Load the configured shaders.

        const std::string& vert_name = root.get_s("vert");
        const std::string& frag_name = root.get_s("frag");

        const char *vert_src = (const char *) ::data->load(vert_name);
        const char *frag_src = (const char *) ::data->load(frag_name);

        // Create the new cache and model.

        cache = new scm_cache(::conf->get_i("scm_viewer_cache_size", 256));
        model = new scm_model(*cache, vert_src, frag_src, n, s);

        ::data->free(vert_name);
        ::data->free(frag_name);

        // Register all frames with the cache.

        for (app::node n = root.find("frame"); n; n = root.next(n, "frame"))
            frame.push_back(new scm_frame(cache, n));

        // If there were no frames, register a flat image set.

        if (frame.empty())
            frame.push_back(new scm_frame(cache, root));

        // Load all land marks.

        for (app::node n = root.find("step"); n; n = root.next(n, "step"))
            mark.push_back(scm_step(n));

        load_label("csv/IAUMOON.csv");

        // Dismiss the GUI.

        gui_state = false;
    }
}

void scm_viewer::cancel()
{
    gui_state = false;
}

void scm_viewer::unload()
{
    std::vector<scm_frame *>::iterator i;

    for (i = frame.begin(); i != frame.end(); ++i)
        delete (*i);

    frame.clear();

    if (label) delete label;
    if (model) delete model;
    if (cache) delete cache;

    label = 0;
    model = 0;
    cache = 0;
}

void scm_viewer::goto_next()
{
    timer_e = floor(timer + 1.0);
    timer_d = +2.0;
}

void scm_viewer::goto_prev()
{
    timer_e = ceil(timer - 1.0);
    timer_d = -2.0;
}

//------------------------------------------------------------------------------

ogl::range scm_viewer::prep(int frusc, const app::frustum *const *frusv)
{
    double r = radius * get_scale(here.get_radius());

    if (cache && model)
    {
        if (::host->get_movie_mode())
            cache->sync(model->tick());
        else
            cache->update(model->tick());
    }
    return ogl::range(0.1, r * 10.0);
}

void scm_viewer::lite(int frusc, const app::frustum *const *frusv)
{
}

void scm_viewer::draw(int frusi, const app::frustum *frusp, int chani)
{
    const double *P =  frusp->get_P();
    const double *M = ::user->get_M();
    const int     w = ::host->get_buffer_w();
    const int     h = ::host->get_buffer_h();

    if (model)
    {
        model->set_debug(debug_bound);

        // Compute the model view matrix to be used for view determination.

        double r = radius * get_scale(here.get_radius());
        double V[16];

        minvert(V, M);
        Rmul_xlt_mat(V, 0, -height, 0);
        Rmul_scl_mat(V, r, r, r);

        // Select the set of files to be drawn and pre-cached.

        tovert.clear();
        tofrag.clear();
        toprep.clear();

        if (timer)
        {
            int a = int(floor(timer)) % frame.size();
            int b =           (a + 1) % frame.size();
            int c =           (b + 1) % frame.size();

            frame[a]->apply(chani, tovert, tofrag);
            frame[b]->apply(chani, tovert, tofrag);
            frame[c]->apply(chani, toprep, toprep);
        }
        else
            frame[0]->apply(chani, tovert, tofrag);

        // Draw the sphere.

        if (debug_wire)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glLineWidth(1.0);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }

        model->set_fade(timer - floor(timer));

        model->draw(P, V, w, h, &tovert.front(), tovert.size(),
                                &tofrag.front(), tofrag.size(),
                                &toprep.front(), toprep.size());

        if (debug_wire)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void scm_viewer::over(int frusi, const app::frustum *frusp, int chani)
{
    double s = get_scale(here.get_radius());
    double r = radius;

    frusp->draw();
   ::user->draw();

    // Draw the label overlay.

    if (label && debug_label)
    {
        glPushMatrix();
        {
            glScaled(s, s, s);
            glScaled(r, r, r);
            label->draw();
        }
        glPopMatrix();
    }

    // Draw the path overlay.

    if (debug_path)
    {
        glPushMatrix();
        {
            glScaled(s, s, s);
            path.draw();
        }
        glPopMatrix();
    }

    // Draw the cache overlay.

    if (cache && debug_cache)
        cache->draw();

    // Draw the GUI overlay.

    if (gui_state)
        gui_draw();
}

//------------------------------------------------------------------------------

bool scm_viewer::process_key(app::event *E)
{
    const int d = E->data.key.d;
    const int k = E->data.key.k;
    const int c = E->data.key.m & KMOD_CTRL;
    const int s = E->data.key.m & KMOD_SHIFT;

    if (d)
    {
        if (!c && !s)
            switch (k)
            {
                case 280: goto_next();                return true; // Page Up
                case 281: goto_prev();                return true; // Page Down

                case 282: gui_state   = !gui_state;   return true; // F1
                case 283: debug_cache = !debug_cache; return true; // F2
                case 284: debug_label = !debug_label; return true; // F3
                case 285: debug_path  = !debug_path;  return true; // F4
                case 286: debug_wire  = !debug_wire;  return true; // F5
                case 287: debug_bound = !debug_bound; return true; // F6

                case 8: cache->flush();               return true; // Backspace
            }

        if (c)
        {
            switch (k)
            {
                case 'c': path.clear();    return true; // ^C
                case 'b': path.back(s);    return true; // ^B
                case 'f': path.fore(s);    return true; // ^F
                case 'p': path.prev();     return true; // ^P
                case 'n': path.next();     return true; // ^N
                case 's': path.save();     return true; // ^S
                case 'l': path.load();     return true; // ^L
                case 'd': path.del();      return true; // ^D
                case 'i': path.ins(here);  return true; // ^I
                case 'a': path.add(here);  return true; // ^A
                case 'o': path.set(here);  return true; // ^O
                case 'r': path.home();     return true; // ^R
                case 'j': path.jump();
                          path.get(here);  return true; // ^J
            }
        }

        if (k == 32) // Space
        {
            path.stop();
            return true;
        }
        if (k == 273) // Up
        {
            if      (c) { path.inc_tens(); return true; }
            else if (s) { path.dec_bias(); return true; }
            else        { path.faster();   return true; }
        }
        if (k == 274) // Down
        {
            if      (c) { path.dec_tens(); return true; }
            else if (s) { path.inc_bias(); return true; }
            else        { path.slower();   return true; }
        }
    }

    return prog::process_event(E);
}

bool scm_viewer::process_user(app::event *E)
{
    // Extract the landmark name from the user event structure.

    char name[sizeof (long long) + 1];

    memset(name, 0, sizeof (name));
    memcpy(name, &E->data.user.d, sizeof (long long));

    // Scan the landmark vector for a matching name.

    for (int i = 0; i < int(mark.size()); i++)
        if (mark[i].get_name().compare(name) == 0)
        {
            // Construct a path from here to there.

            scm_step src = here;
            scm_step dst = mark[i];
            scm_step mid(&src, &dst, 0.5);

            if (!path.playing())
                src.set_speed(0.05);

            mid.set_speed(1.00);
            dst.set_speed(0.05);

            path.stop();
            path.clear();
            path.add(src);
            path.add(mid);
            path.add(dst);
            path.home();
            path.fore(false);

            // Switch the labels.

            load_label(mark[i].get_label());

            return true;
        }

    return false;
}

bool scm_viewer::process_tick(app::event *E)
{
    if (path.playing())
    {
        path.time(E->data.tick.dt / 1000.0);
        path.get(here);
    }

    timer += timer_d * E->data.tick.dt / 1000.0;

    if (timer_d > 0.0 && timer > timer_e)
    {
        timer   = timer_e;
        timer_d = 0;
    }
    if (timer_d < 0.0 && timer < timer_e)
    {
        timer   = timer_e;
        timer_d = 0;
    }

    return false;
}

bool scm_viewer::process_event(app::event *E)
{
    if      (E->get_type() == E_KEY)  return process_key(E);
    else if (E->get_type() == E_USER) return process_user(E);
    else if (E->get_type() == E_TICK) return process_tick(E);

    // Pass the event to the GUI if visible.

    else if (gui_state)
    {
        switch (E->get_type())
        {
            case E_CLICK: return gui_click(E);
            case E_POINT: return gui_point(E);
            case E_KEY:   return gui_key  (E);
        }
    }

    return false;
}

//------------------------------------------------------------------------------

void scm_viewer::load_label(const std::string& name)
{
    // Load the CSV file.

    const void *data_ptr;
    size_t      data_len;

    data_ptr = ::data->load(name, &data_len);

    // Release the previous label, if any.

    if (label) delete label;

    // Create the new label.

    label = new scm_label(data_ptr, data_len, font_ptr, font_len);

    // Release the CSV file.

    ::data->free(name);
}

//------------------------------------------------------------------------------

void scm_viewer::gui_init()
{
    const app::frustum *overlay = ::host->get_overlay();

    int w = overlay ? overlay->get_pixel_w() : ::host->get_buffer_w();
    int h = overlay ? overlay->get_pixel_h() : ::host->get_buffer_h();

    gui_state = true;

    ui = new scm_loader(this, w, h);
}

void scm_viewer::gui_free()
{
    delete ui;
}

void scm_viewer::gui_draw()
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        if (cache)
            ui->set_status(cache->get_size());

        glEnable(GL_DEPTH_CLAMP_NV);
        {
            overlay->draw();
            overlay->overlay();
            ui->draw();
        }
        glDisable(GL_DEPTH_CLAMP_NV);
    }
}

bool scm_viewer::gui_point(app::event *E)
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        int x;
        int y;

        overlay->pointer_to_2D(E, x, y);
        ui->point(x, y);

        return true;
    }
    return false;
}

bool scm_viewer::gui_click(app::event *E)
{
    ui->click(E->data.click.m,
              E->data.click.d);
    return true;
}

bool scm_viewer::gui_key(app::event *E)
{
    if (E->data.key.d)
    {
        ui->key(E->data.key.c,
                E->data.key.k,
                E->data.key.m);
    }
    return true;
}

//------------------------------------------------------------------------------

