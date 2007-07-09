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

#include "opengl.hpp"
#include "mode.hpp"

//-----------------------------------------------------------------------------

bool mode::mode::point(const double *, const double *, int x, int y)
{
    int dx = x - drag_x;
    int dy = y - drag_y;

    drag_x = x;
    drag_y = y;

    if (drag_d && (SDL_GetModState() & KMOD_ALT))
    {
        world.mov_light(dx, dy);
        return true;
    }

    return false;
}

bool mode::mode::click(int b, bool d)
{
    if (b == 3)
    {
        drag_d = d;
        return true;
    }

    return false;
}

bool mode::mode::keybd(int, bool, int)
{
    return false;
}

bool mode::mode::timer(double dt)
{
    return false;
}

//-----------------------------------------------------------------------------

double mode::mode::view(const double *planes)
{
    return world.view(true, planes);
}

void mode::mode::draw(const double *points)
{
    world.draw(true, points);
}

//-----------------------------------------------------------------------------
