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

#include <SDL.h>

#include "host.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

mode::info::info(wrl::world& w) : mode(w), gui(0)
{
    int gui_w;
    int gui_h;

    ::host->gui_size(gui_w, gui_h);

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

bool mode::info::point(const double *p, const double *v)
{
    int x;
    int y;

    ::host->gui_pick(x, y, p, v);

    gui->point(x, y);
    return false;
}

bool mode::info::click(int b, bool d)
{
    if (b == 1)
    {
        gui->click(d);
        return true;
    }
    return false;
}

bool mode::info::keybd(int k, bool d, int c)
{
    if (d)
    {
        gui->keybd(k, c);
    }
    return true;
}

//-----------------------------------------------------------------------------

bool mode::info::timer(double dt)
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
