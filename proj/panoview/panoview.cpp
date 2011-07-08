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

#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>

#include "math3d.h"
#include "panoview.hpp"

//------------------------------------------------------------------------------

panoview::panoview(const std::string& tag) : app::prog(tag), C(0), L(0), spin(0)
{
    curr_zoom   = 1.0;
    drag_state  = false;
    
    debug_zoom  = false;
    debug_cache = false;
    debug_color = false;
    
    gui_init();
}

panoview::~panoview()
{
    gui_free();
    
    if (L) delete L;
    if (C) delete C;
}

//------------------------------------------------------------------------------

void panoview::load(const std::string& name)
{
    // If the named file exists and contains an XML panorama definition...
    
    app::file file(name);

    if (app::node root = file.get_root().find("panorama"))
    {
        // Clear out the existing data.
        
        if (L) delete L;
        if (C) delete C;
        pan.clear();

        // Parse the panorama configuration
        
        const std::string& vert_name = root.get_s("vert");
        const std::string& frag_name = root.get_s("frag");
        
        int n = root.get_i("mesh",  16);
        int d = root.get_i("depth",  8);
        int s = root.get_i("size", 512);
        
        // Create the new cache and model.
        
        C = new sph_cache(::conf->get_i("panoview_cache_size", 256));
        L = new sph_model(vert_name, frag_name, *C, n, d, s);

        // Load all images.

        for (app::node n = root.find("image"); n; n = root.next(n, "image"))
        {
            const std::string& name = n.get_s("file");
            pan.push_back(C->add_file(name));
        }

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

    if (C && L)
    {
        double V[16];

        minvert(V, M);

        if (debug_zoom)
            L->set_zoom(  0.0,   0.0,   -1.0, pow(curr_zoom, 1.2));
        else
            L->set_zoom(-M[8], -M[9], -M[10], pow(curr_zoom, 1.2));

        C->set_debug(debug_color);

        L->prep(P, V, w, h);
        L->draw(P, V, &pan.front(), int(pan.size()));
    }
    
    if (C && debug_cache)
        C->draw();
        
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
        ::user->look(spin * E->data.tick.dt / 100.0, 0.0);

    return prog::process_event(E);
}

//------------------------------------------------------------------------------

bool panoview::pan_point(app::event *E)
{
    if (const app::frustum *overlay = ::host->get_overlay())
        overlay->pointer_to_2D(E, curr_x, curr_y);
            
    if (drag_state)
    {
        curr_zoom = drag_zoom + (curr_y - drag_y) / 300.0f;
        
        if (curr_zoom <   0.5) curr_zoom =   0.5;
        if (curr_zoom >  10.0) curr_zoom =  10.0;

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
    if (E->data.key.d)
        switch (E->data.key.k)
        {
        case '0': spin = 0; return true;
        case '1': spin = 1; return true;
        case '2': spin = 2; return true;
        case '3': spin = 3; return true;
        case '4': spin = 4; return true;
        case '5': spin = 5; return true;
        case '6': spin = 6; return true;
        case '7': spin = 7; return true;
        case '8': spin = 8; return true;
        case '9': spin = 9; return true;
        
        case 282 : gui_state   = !gui_state;   return true;
        case 283 : debug_cache = !debug_cache; return true;
        case 284 : debug_color = !debug_color; return true;
        case 285 : debug_zoom  = !debug_zoom;  return true;
        
        case 13  : curr_zoom = 1.0; return true;
        case 8   : C->flush();      return true;
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
