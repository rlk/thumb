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

#include <app-data.hpp>
#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>

#include "math3d.h"
#include "panoview.hpp"

//------------------------------------------------------------------------------

panoview::panoview(const std::string& tag) : app::prog(tag),
    channel (0),
    cache   (0),
    model   (0),
    channels(0),
    spin    (0),
    time    (0),
    dtime   (0)
{
    curr_zoom   = 0.0;
    drag_state  = false;
    
    debug_zoom  = false;
    debug_cache = false;
    debug_color = false;
    
    gui_init();

    TIFFSetWarningHandler(0);
}

panoview::~panoview()
{
    gui_free();
    
    if (channel) delete [] channel;
    if (model)   delete    model;
    if (cache)   delete    cache;
}

//------------------------------------------------------------------------------

void panoview::load(const std::string& name)
{
    // If the named file exists and contains an XML panorama definition...
    
    app::file file(name);

    if (app::node root = file.get_root().find("panorama"))
    {
        // Clear out the existing data.
        
        if (channel) delete [] channel;
        if (model)   delete    model;
        if (cache)   delete    cache;

        // Parse the panorama configuration
        
        channels = root.get_i("channels", 1);
        int    d = root.get_i("depth",    8);
        int    n = root.get_i("mesh",    16);
        int    s = root.get_i("size",   512);

        // Load the configured shaders.
        
        const std::string& vert_name = root.get_s("vert");
        const std::string& frag_name = root.get_s("frag");
        
        const char *vert_src = (const char *) ::data->load(vert_name);
        const char *frag_src = (const char *) ::data->load(frag_name);
        
        // Create the new cache and model.
        
        cache   = new sph_cache(::conf->get_i("panoview_cache_size", 256));
        model   = new sph_model(*cache, vert_src, frag_src, n, d, s);
        channel = new panochan[channels];

        // Register all images with the cache.

        for (app::node n = root.find("image"); n; n = root.next(n, "image"))
        {
            const std::string& name = n.get_s("file");
            int                   i = n.get_i("channel", 0);
            
            if (0 <= i && i < channels)
                channel[i].add(cache->add_file(name));
        }

        ::data->free(vert_name);
        ::data->free(frag_name);

        gui_state = false;
    }
}

void panoview::cancel()
{
    gui_state = false;
}

//------------------------------------------------------------------------------

ogl::range panoview::prep(int frusc, const app::frustum *const *frusv)
{
    // This will need to change on a multi-pipe system.
    
    if (cache && model)
    {
        cache->update(model->tick());
    }
    return ogl::range(0.001, 10.0);
}

void panoview::lite(int frusc, const app::frustum *const *frusv)
{
}

void panoview::draw(int frusi, const app::frustum *frusp)
{
    const double *P =  frusp->get_P();
    const double *M = ::user->get_M();
    const int     w = ::host->get_buffer_w();
    const int     h = ::host->get_buffer_h();

    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    if (cache && model)
    {
        double V[16];
        
        minvert(V, M);

        if (debug_zoom)
            model->set_zoom(  0.0,   0.0,   -1.0, pow(10.0, curr_zoom));
        else
            model->set_zoom(-M[8], -M[9], -M[10], pow(10.0, curr_zoom));

        cache->set_debug(debug_color);

        if (time)
        {
            int fv[2];
            int pv[2];
            int pc = 0;

            fv[0] = channel[frusi % channels].get(int(floor(time)));
            fv[1] = channel[frusi % channels].get(int( ceil(time)));
            
            if (dtime < 0)
            {
                pv[0] = channel[frusi % channels].get(int(floor(time)) - 1);
                pc    = 1;
            }
            else
            {
                pv[0] = channel[frusi % channels].get(int( ceil(time)) + 1);
                pc    = 1;
            }
                
            model->set_fade(time - floor(time));
            model->prep(P, V, w, h);
            model->draw(P, V, fv, 2, pv, pc);
        }
        else
        {
            int f = channel[frusi % channels].get(0);

            model->set_fade(0);
            model->prep(P, V, w, h);
            model->draw(P, V, &f, 1, 0, 0);
        }
    }
    
    if (cache && debug_cache)
        cache->draw();
        
    gui_draw();
}

//------------------------------------------------------------------------------

bool panoview::process_event(app::event *E)
{
    if (E->get_type() == E_CLICK)
        return (gui_click(E) || pan_click(E) || prog::process_event(E));

    if (E->get_type() == E_POINT)
        return (gui_point(E) || pan_point(E) || prog::process_event(E));

    if (E->get_type() == E_KEY)
        return (gui_key(E)   || pan_key(E)   || prog::process_event(E));

    if (E->get_type() == E_TICK)
    {
        const double dt = E->data.tick.dt / 1000.0;

        ::user->look(spin * dt, 0.0);
        time +=     dtime * dt;
    }

    return prog::process_event(E);
}

//------------------------------------------------------------------------------

bool panoview::pan_point(app::event *E)
{
    if (const app::frustum *overlay = ::host->get_overlay())
        overlay->pointer_to_2D(E, curr_x, curr_y);
            
    if (drag_state)
    {
        curr_zoom = drag_zoom + (curr_y - drag_y) / 500.0f;
        
        if (curr_zoom < -2.0) curr_zoom = -2.0;
        if (curr_zoom >  0.5) curr_zoom =  0.5;

        return true;
    }
    return false;
}

bool panoview::pan_click(app::event *E)
{
    if (E->data.click.b == 1)
    {
        drag_state = E->data.click.d;

        drag_x     = curr_x;
        drag_y     = curr_y;
        drag_zoom  = curr_zoom;
        
        return true;
    }
    return false;
}

bool panoview::pan_key(app::event *E)
{
    int sh = (E->data.key.m & 3);
    
    if (E->data.key.d)
        switch (E->data.key.k)
        {
        case '0': if (sh) spin = 0; else time = 0; return true;
        case '1': if (sh) spin = 1; else time = 1; return true;
        case '2': if (sh) spin = 2; else time = 2; return true;
        case '3': if (sh) spin = 3; else time = 3; return true;
        case '4': if (sh) spin = 4; else time = 4; return true;
        case '5': if (sh) spin = 5; else time = 5; return true;
        case '6': if (sh) spin = 6; else time = 6; return true;
        case '7': if (sh) spin = 7; else time = 7; return true;
        case '8': if (sh) spin = 8; else time = 8; return true;
        case '9': if (sh) spin = 9; else time = 9; return true;
        
        case 273: dtime =  0.0; return true;
        case 274: dtime =  0.0; return true;
        case 275: dtime =  0.5; return true;
        case 276: dtime = -0.5; return true;
        
        case 282 : gui_state   = !gui_state;   return true;
        case 283 : debug_cache = !debug_cache; return true;
        case 284 : debug_color = !debug_color; return true;
        case 285 : debug_zoom  = !debug_zoom;  return true;
        
        case 13  : curr_zoom = 0; return prog::process_event(E);
        case 8   : cache->flush();    return true;
        }
    
    return false;
}

//------------------------------------------------------------------------------

void panoview::gui_init()
{
    const app::frustum *overlay = ::host->get_overlay();

    int w = overlay ? overlay->get_pixel_w() : ::host->get_buffer_w();
    int h = overlay ? overlay->get_pixel_h() : ::host->get_buffer_h();

    gui = new loader(this, w, h);
    gui_state = true;
}

void panoview::gui_free()
{
    delete gui;
}

void panoview::gui_draw()
{
    if (gui_state)
    {
        if (const app::frustum *overlay = ::host->get_overlay())
        {
            glEnable(GL_DEPTH_CLAMP_NV);
            {
                overlay->draw();
                overlay->overlay();
                gui->draw();
            }
            glDisable(GL_DEPTH_CLAMP_NV);
        }
    }
}

bool panoview::gui_point(app::event *E)
{
    if (gui_state)
    {
        if (const app::frustum *overlay = ::host->get_overlay())
        {
            int x;
            int y;
            
            overlay->pointer_to_2D(E, x, y);
            gui->point(x, y);
            
            return true;
        }
    }
    return false;
}

bool panoview::gui_click(app::event *E)
{
    if (gui_state)
    {
        gui->click(E->data.click.m,
                   E->data.click.d);
        return true;
    }
    return false;
}

bool panoview::gui_key(app::event *E)
{
    if (gui_state)
    {
        if (E->data.key.d)
        {
            if (E->data.key.k == 282)
                gui_state = !gui_state;
            else
                gui->key(E->data.key.c,
                         E->data.key.k,
                         E->data.key.m);
        }
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
