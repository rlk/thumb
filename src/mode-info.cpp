//  Copyright (C) 2005 Robert Kooima
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

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "mode-info.hpp"
#include "gui-control.hpp"

//-----------------------------------------------------------------------------

mode::info::info(wrl::world *w) : mode(w), gui(0)
{
    int gui_w = 1024;
    int gui_h =  768;

    /* TODO: make this seek the "right" size */

    gui = new cnt::control(w, gui_w, gui_h);
}

//-----------------------------------------------------------------------------

double mode::info::prep(int frusc, app::frustum **frusv)
{
    assert(world);

    return world->prep(frusc, frusv, true);
}

void mode::info::draw(int frusi, app::frustum *frusp)
{
    assert(world);
    assert(gui);

    world->draw(frusi, frusp, true);
    gui->draw();
}

//-----------------------------------------------------------------------------

bool mode::info::process_event(app::event *E)
{
    assert(gui);
/* TODO
    return ( gui->process_event(E) ||
            mode::process_event(E));
*/
    return mode::process_event(E);
}

//-----------------------------------------------------------------------------
