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

#include <cassert>

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <mode-info.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <gui-control.hpp>

//-----------------------------------------------------------------------------

mode::info::info(wrl::world *w) :
    mode(w),
    gui_w(::host->get_window_w()),
    gui_h(::host->get_window_h()),
    pane(0), gui(0)
{
}

mode::info::~info()
{
    if (gui) delete gui;
}

//-----------------------------------------------------------------------------

void mode::info::gui_show()
{
    gui = new cnt::control(world, gui_w, gui_h);
    gui->set_index(pane);
    gui->show();
}

void mode::info::gui_hide()
{
    gui->hide();
    pane = gui->get_index();
    delete gui;
    gui = 0;
}

//-----------------------------------------------------------------------------

ogl::aabb mode::info::prep(int frusc, const app::frustum *const *frusv)
{
    assert(world);

    ogl::aabb b;

    b.merge(world->prep_fill(frusc, frusv));
    b.merge(world->prep_line(frusc, frusv));

    return b;
}

void mode::info::draw(int frusi, const app::frustum *frusp)
{
    assert(world);

    // Draw the world.

     frusp->load_transform();
    ::view->load_transform();

    world->draw_fill(frusi, frusp);
    world->draw_line();

    // Draw the GUI.

    const app::frustum *overlay = ::host->get_overlay();

    if (overlay == 0)
        overlay = frusp;

    glEnable(GL_DEPTH_CLAMP_NV);
    {
        const vec3 *p = overlay->get_corners();

        const double w = length(p[1] - p[0]) / gui_w;
        const double h = length(p[2] - p[0]) / gui_h;
        const vec3   x = normal(p[1] - p[0]);
        const vec3   y = normal(p[2] - p[0]);
        const vec3   z = normal(cross(x, y));

        mat4 T = translation(p[0])
               *   transpose(mat3(x, y, z))
               *       scale(vec3(w, h, 1));

        glLoadMatrixd(transpose(T));
        gui->draw();
    }
    glDisable(GL_DEPTH_CLAMP_NV);
}

//-----------------------------------------------------------------------------

bool mode::info::process_event(app::event *E)
{
    const app::frustum *overlay = ::host->get_overlay();
    double x = 0;
    double y = 0;

    assert(E);

    // Translate event data into GUI method calls.

    switch (E->get_type())
    {
    case E_POINT:

        if (gui && E->data.point.i == 0)
        {
            if (overlay)
                overlay->pointer_to_2D(E, x, y);

            gui->point(toint(x * gui_w),
                       toint(y * gui_h));

            return true;
        }
        return false;

    case E_CLICK:

        if (gui && E->data.click.b == 1)
        {
            gui->click(E->data.click.m,
                       E->data.click.d);
            return true;
        }
        return false;

    case E_KEY:

        if (gui && E->data.key.d)
        {
            gui->key(E->data.key.k,
                     E->data.key.m);
        }
        return true;

    case E_TEXT:

        if (gui)
            gui->glyph(E->data.text.c);
        return true;

    case E_START:

        SDL_StartTextInput();
        gui_show();
        return false;

    case E_CLOSE:

        SDL_StopTextInput();
        gui_hide();
        return false;
    }

    return mode::process_event(E);
}

//-----------------------------------------------------------------------------
