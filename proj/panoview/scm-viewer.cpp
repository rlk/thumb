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
    debug_color(false),
    debug_label(false),
    debug_wire (true)
{
    TIFFSetWarningHandler(0);
    gui_init();
}

scm_viewer::~scm_viewer()
{
    gui_free();
    unload();
}

//------------------------------------------------------------------------------

scm_frame::scm_frame(scm_cache *cache, app::node node)
{
    for (app::node n = node.find("scm"); n; n = node.next(n, "scm"))
    {
        image I;

        I.file = cache->add_file(n.get_s("file"),
                                 n.get_f("r0", 0.0),
                                 n.get_f("r1", 1.0),
                                 n.get_i("overdraw", 0));

        I.shader  = (n.get_s("shader") == "vert") ? 0 : 1;
        I.channel = (n.get_i("channel"));

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

        int    d  = root.get_i("depth",  8);
        int    n  = root.get_i("mesh",  16);
        int    s  = root.get_i("size", 512);
        // double r0 = root.get_f("r0",   1.0);
        // double r1 = root.get_f("r1",   1.0);

        height = root.get_f("height", 0.0);
        radius = root.get_f("radius", 6.0);

        // Load the configured shaders.

        const std::string& vert_name = root.get_s("vert");
        const std::string& frag_name = root.get_s("frag");

        const char *vert_src = (const char *) ::data->load(vert_name);
        const char *frag_src = (const char *) ::data->load(frag_name);

        // Create the new cache and model.

        cache = new scm_cache(::conf->get_i("scm_viewer_cache_size", 128));
        model = new scm_model(*cache, vert_src, frag_src, n, s, d);//, r0, r1);

        // Register all frames with the cache.

        for (app::node n = root.find("frame"); n; n = root.next(n, "frame"))
            frame.push_back(new scm_frame(cache, n));

        // If there were no frames, register a flat image set.

        if (frame.empty())
            frame.push_back(new scm_frame(cache, root));

        // Load the label.

        font_ptr = ::data->load(::conf->get_s("sans_font"), &font_len);
        data_ptr = ::data->load("IAUMOON.csv");

        label = new scm_label(data_ptr, data_len, font_ptr, font_len);

        ::data->free(vert_name);
        ::data->free(frag_name);

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
    // This will need to change on a multi-pipe system.

    if (cache && model)
    {
        cache->update(model->tick());
    }
    return ogl::range(0.1, radius * 10.0);
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

    if (cache)
        cache->set_debug(debug_color);

    if (model)
    {
        // Compute the model view matrix to be used for view determination.

        double V[16];

        minvert(V, M);
        Rmul_xlt_mat(V,      0, -height,      0);
        Rmul_scl_mat(V, radius,  radius, radius);

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
        }

        model->set_fade(timer - floor(timer));
#if 0
        model->prep(P, V, w, h);
        model->draw(P, V, &tovert.front(), tovert.size(),
                          &tofrag.front(), tofrag.size(),
                          &toprep.front(), toprep.size());
#else
        model->draw(P, V, w, h, &tovert.front(), tovert.size(),
                                &tofrag.front(), tofrag.size(),
                                &toprep.front(), toprep.size());
#endif

        if (debug_wire)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void scm_viewer::over(int frusi, const app::frustum *frusp, int chani)
{
    const double *M = ::user->get_M();

    // Draw the label overlay.

    if (label && debug_label)
    {
        double altitude = sqrt(DOT3(M + 12, M + 12));
        double position[3];

        position[0] = M[12] / altitude;
        position[1] = M[13] / altitude;
        position[2] = M[14] / altitude;

        frusp->draw();
       ::user->draw();

        glPushMatrix();
        {
            glScaled(radius, radius, radius);
            label->draw(position, radius, altitude);
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

bool scm_viewer::process_event(app::event *E)
{
    // Toggle global options in response to function keys.

    if (E->get_type() == E_KEY)
    {
        if (E->data.key.d)
        {
            switch (E->data.key.k)
            {
                case 282 : gui_state   = !gui_state;   return true;
                case 283 : debug_cache = !debug_cache; return true;
                case 284 : debug_label = !debug_label; return true;
                case 285 : debug_wire  = !debug_wire;  return true;
                case 8   : cache->flush();             return true;
            }
        }
    }

    // Pass the event to the GUI if visible.

    if (gui_state)
    {
        switch (E->get_type())
        {
            case E_CLICK: return gui_click(E);
            case E_POINT: return gui_point(E);
            case E_KEY:   return gui_key  (E);
        }
    }

    // Apply the passage of time to animation playback.

    if (E->get_type() == E_TICK)
    {
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
    }

    if (E->get_type() == E_KEY)
        return prog::process_event(E);
    else
        return false;
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
