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
#include "view-app.hpp"

//------------------------------------------------------------------------------

view_app::view_app(const std::string& exe,
                       const std::string& tag) : app::prog(exe, tag),
    bound  (0),
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

view_app::~view_app()
{
    gui_free();
    unload();

    ::data->free(::conf->get_s("sans_font"));
}

//------------------------------------------------------------------------------

void view_app::load_model(app::node p)
{
    // Load the configured shaders.

    const std::string& vert_name = p.get_s("vert");
    const std::string& frag_name = p.get_s("frag");

    const char *vert_src = (const char *) ::data->load(vert_name);
    const char *frag_src = (const char *) ::data->load(frag_name);

    // Parse the sphere configuration

    int mesh = p.get_i("mesh",  16);
    int size = p.get_i("size", 512);

    // Create the model.

    model = new scm_model(vert_src, frag_src, mesh, size);

    // Release the shader source.

    ::data->free(vert_name);
    ::data->free(frag_name);
}

void view_app::load_caches(app::node p)
{
    int size = ::conf->get_i("scm_cache_size", 64);

    // Create a new cache object for each node.

    for (app::node i = p.find("cache"); i; i = p.next(i, "cache"))
    {
        int    n  = i.get_i("n",     256);
        int    c  = i.get_i("c",       3);
        int    b  = i.get_i("b",       1);
        int    t  = i.get_i("threads", 2);
        double r0 = i.get_f("r0",    0.0);
        double r1 = i.get_f("r1",    1.0);

        caches.push_back(new scm_cache(size, n, c, b, t, r0, r1));
    }
}

void view_app::load_images(app::node p, scm_frame *f)
{
    // Create a new image object for each node.

    for (app::node i = p.find("image"); i; i = p.next(i, "image"))
    {
        const std::string& name = i.get_s("name");
        const std::string& scm  = i.get_s("scm");
        int                cc   = i.get_i("cache",    0);
        int                ch   = i.get_i("channel", -1);
        int                ht   = i.get_i("height",   0);

        if (0 <= cc && cc < int(caches.size()))
        {
            if (ht)
            {
                f->add_image(new scm_image(name, scm, caches[cc], ch, true));
                bound = caches[cc];
            }
            else
            {
                f->add_image(new scm_image(name, scm, caches[cc], ch, false));
            }
        }
    }
}

void view_app::load_frames(app::node p)
{
    // Create a new frame object for each node.

    for (app::node i = p.find("frame"); i; i = p.next(i, "frame"))
    {
        scm_frame *f = new scm_frame();

        load_images(i, f);
        frames.push_back(f);
    }
}

void view_app::load_steps(app::node p)
{
    // Create a new step object for each node.

    for (app::node i = p.find("step"); i; i = p.next(i, "step"))
        steps.push_back(view_step(i));
}

void view_app::load(const std::string& name)
{
    // If the named file exists and contains an XML panorama definition...

    app::file file(name);

    if (app::node root = file.get_root().find("sphere"))
    {
        // Clear out the existing data.

        unload();

        // Configure the viewer.

        height = root.get_f("height", 0.0);
        radius = root.get_f("radius", 6.0);

        // Load all data.

        load_model (root);
        load_caches(root);
        load_frames(root);
        load_steps (root);
        load_label ("csv/IAUMOON.csv");

        // Dismiss the GUI.

        gui_state = false;
    }
}

void view_app::cancel()
{
    gui_state = false;
}

void view_app::unload()
{
    for (scm_cache_i i = caches.begin(); i != caches.end(); ++i) delete (*i);
    for (scm_frame_i j = frames.begin(); j != frames.end(); ++j) delete (*j);

    caches.clear();
    frames.clear();

    if (label) delete label;
    if (model) delete model;

    label = 0;
    model = 0;
}

void view_app::goto_next()
{
    timer_e = floor(timer + 1.0);
    timer_d = +2.0;
}

void view_app::goto_prev()
{
    timer_e = ceil(timer - 1.0);
    timer_d = -2.0;
}

//------------------------------------------------------------------------------

ogl::range view_app::prep(int frusc, const app::frustum *const *frusv)
{
    double r = radius * get_scale(here.get_radius());

    if (model)
    {
        if (::host->get_movie_mode())
            for (scm_cache_i i = caches.begin(); i != caches.end(); ++i)
                (*i)->sync  (model->tick());
        else
            for (scm_cache_i i = caches.begin(); i != caches.end(); ++i)
                (*i)->update(model->tick());
    }
    return ogl::range(0.1, r * 10.0);
}

void view_app::lite(int frusc, const app::frustum *const *frusv)
{
}

void view_app::draw(int frusi, const app::frustum *frusp, int chani)
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

        // Draw the sphere.

        if (debug_wire)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glLineWidth(1.0);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
        }

        frames[0]->set_channel(chani);
        model->draw(frames[0], P, V, w, h);

        if (debug_wire)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void view_app::over(int frusi, const app::frustum *frusp, int chani)
{
    double s = get_scale(here.get_radius());
    double r = radius;

    frusp->draw();
   ::user->draw();

    // Draw the label overlay.

    if (debug_label)
    {
        glPushMatrix();
        {
            glScaled(s, s, s);
            glScaled(r, r, r);

            if (label)
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

    if (debug_cache)
    {
        if (!caches.empty())
            caches[0]->draw();
    }

    // Draw the GUI overlay.

    if (gui_state)
        gui_draw();
}

//------------------------------------------------------------------------------

bool view_app::process_key(app::event *E)
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

                case 8:
                    for (scm_cache_i i = caches.begin(); i != caches.end(); ++i)
                        (*i)->flush();
                    return true; // Backspace
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

bool view_app::process_user(app::event *E)
{
    // Extract the landmark name from the user event structure.

    char name[sizeof (long long) + 1];

    memset(name, 0, sizeof (name));
    memcpy(name, &E->data.user.d, sizeof (long long));

    // Scan the landmark vector for a matching name.

    for (int i = 0; i < int(steps.size()); i++)
        if (steps[i].get_name().compare(name) == 0)
        {
            // Construct a path from here to there.

            view_step src = here;
            view_step dst = steps[i];
            view_step mid(&src, &dst, 0.5);

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

            load_label(steps[i].get_label());

            return true;
        }

    return false;
}

bool view_app::process_tick(app::event *E)
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

bool view_app::process_event(app::event *E)
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

void view_app::load_label(const std::string& name)
{
    // Load the CSV file.

    const void *data_ptr;
    size_t      data_len;

    data_ptr = ::data->load(name, &data_len);

    // Release the previous label, if any.

    if (label) delete label;

    // Create the new label.

    label = new scm_label(data_ptr, data_len, font_ptr, font_len,
                    radius, ::conf->get_i("orbiter_icon_size", 16));

    // Release the CSV file.

    ::data->free(name);
}

//------------------------------------------------------------------------------

// Initialize the file selection GUI.

void view_app::gui_init()
{
    const app::frustum *overlay = ::host->get_overlay();

    int w = overlay ? overlay->get_pixel_w() : ::host->get_buffer_w();
    int h = overlay ? overlay->get_pixel_h() : ::host->get_buffer_h();

    gui_state = true;

    ui = new view_load(this, w, h);
}

// Release the file selection GUI.

void view_app::gui_free()
{
    delete ui;
}

// Draw the file selection GUI in the overlay.

void view_app::gui_draw()
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        glEnable(GL_DEPTH_CLAMP_NV);
        {
            overlay->draw();
            overlay->overlay();
            ui->draw();
        }
        glDisable(GL_DEPTH_CLAMP_NV);
    }
}

// Handle a mouse point event while the GUI is visible.

bool view_app::gui_point(app::event *E)
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

// Handle a mouse click event while the GUI is visible.

bool view_app::gui_click(app::event *E)
{
    ui->click(E->data.click.m,
              E->data.click.d);
    return true;
}

// Handle a keyboard event while the GUI is visible.

bool view_app::gui_key(app::event *E)
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

