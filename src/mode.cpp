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
#include "host.hpp"

//-----------------------------------------------------------------------------

bool mode::mode::point(int i, const double *p, const double *q)
{
    return false;
}

bool mode::mode::click(int i, int b, int m, bool d)
{
    return false;
}

bool mode::mode::keybd(int c, int k, int m, bool d)
{
    return false;
}

bool mode::mode::timer(int t)
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
