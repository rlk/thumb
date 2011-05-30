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
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <gui-control.hpp>

//-----------------------------------------------------------------------------

mode::info::info(wrl::world *w) : mode(w), gui(0)
{
    const app::frustum *overlay = ::host->get_overlay();

    gui = new cnt::control(w, overlay->get_pixel_w(),
                              overlay->get_pixel_h());
}

mode::info::~info()
{
    assert(gui);
    delete gui;
}

//-----------------------------------------------------------------------------

ogl::range mode::info::prep(int frusc, const app::frustum *const *frusv)
{
    assert(world);

    ogl::range r;

    r.merge(world->prep_fill(frusc, frusv));
    r.merge(world->prep_line(frusc, frusv));

    return r;
}

void mode::info::draw(int frusi, const app::frustum *frusp)
{
    const app::frustum *overlay = ::host->get_overlay();

    assert(world);
    assert(gui);

     frusp->draw();
    ::user->draw();

    world->draw_fill(frusi, frusp);
    world->draw_line(frusi, frusp);

    if (overlay)
    {
        glEnable(GL_DEPTH_CLAMP_NV);
        overlay->overlay();
        gui->draw();
        glDisable(GL_DEPTH_CLAMP_NV);
    }
}

//-----------------------------------------------------------------------------

bool mode::info::process_event(app::event *E)
{
    const app::frustum *overlay = ::host->get_overlay();
    int x = 0;
    int y = 0;

    assert(E);
    assert(gui);

    ::host->post_draw();

    // Translate event data into GUI method calls.

    switch (E->get_type())
    {
    case E_POINT:

        if (E->data.point.i == 0)
        {
            if (overlay)
                overlay->pointer_to_2D(E, x, y);

            gui->point(x, y);
            return true;
        }
        return false;

    case E_CLICK:
 
        if (E->data.click.b == SDL_BUTTON_LEFT)
        {
            gui->click(E->data.click.m,
                       E->data.click.d);
            return true;
        }
        return false;

    case E_KEY:

        if (E->data.key.d)
            gui->key(E->data.key.c,
                       E->data.key.k,
                       E->data.key.m);
        return true;

    case E_START:

        host->post_draw();
        gui->show();
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                            SDL_DEFAULT_REPEAT_INTERVAL);
        return false;

    case E_CLOSE:

        SDL_EnableKeyRepeat(0, 0);
        gui->hide();
        host->post_draw();
        return false;
    }

    return mode::process_event(E);
}

//-----------------------------------------------------------------------------
