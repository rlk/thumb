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

#include "play.hpp"
#include "main.hpp"

//-----------------------------------------------------------------------------

mode::play::play(wrl::world& w) : mode(w)
{
}

//-----------------------------------------------------------------------------

void mode::play::enter()
{
    world.play_init();
    clr_time();
}

void mode::play::leave()
{
    world.play_fini();
}

//-----------------------------------------------------------------------------

bool mode::play::timer(int t)
{
    world.play_step(t / 1000.0);
    return true;
}

double mode::play::view(const double *planes)
{
    return world.view(false, planes);
}

void mode::play::draw(const double *points)
{
    world.draw(false, points);
}

//-----------------------------------------------------------------------------
