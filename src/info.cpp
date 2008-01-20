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

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "host.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

mode::info::info(wrl::world& w) : mode(w), gui(0)
{
    int gui_w = 1024;
    int gui_h =  768;

    /* TODO: make this seek the "right" size
    ::host->gui_size(gui_w, gui_h);
    */

    gui = new cnt::control(w, gui_w, gui_h);
}

//-----------------------------------------------------------------------------

void mode::info::enter()
{
    gui->show();
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
                        SDL_DEFAULT_REPEAT_INTERVAL);
}

void mode::info::leave()
{
    SDL_EnableKeyRepeat(0, 0);
    gui->hide();
}

//-----------------------------------------------------------------------------

bool mode::info::point(int i, const double *p, const double *q)
{
    int x = 0;
    int y = 0;

/* TODO: fix GUI transform
    ::host->gui_pick(x, y, p, v);
*/

    gui->point(x, y);
    return false;
}

bool mode::info::click(int i, int b, int m, bool d)
{
    if (b == SDL_BUTTON_LEFT)
    {
        gui->click(m, d);
        return true;
    }
    return false;
}

bool mode::info::keybd(int c, int k, int m, bool d)
{
    if (d) gui->keybd(k, c, m);

    return true;
}

//-----------------------------------------------------------------------------

bool mode::info::timer(int t)
{
    return false;
}

double mode::info::view(const double *planes)
{
    return world.view(true, planes);
}

void mode::info::draw(const double *points)
{
    world.draw(true, points);
    gui->draw();
}

//-----------------------------------------------------------------------------
