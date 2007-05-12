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

mode::play::play(wrl::world& w, ops::scene& s) : mode(w, s)
{
}

//-----------------------------------------------------------------------------

void mode::play::enter()
{
    scene.embody_all();
    clr_time();
}

void mode::play::leave()
{
    scene.debody_all();
}

//-----------------------------------------------------------------------------

bool mode::play::timer(float dt)
{
    scene.step(dt);

    return true;
}

//-----------------------------------------------------------------------------

void mode::play::draw()
{
    scene.draw_scene();
}

//-----------------------------------------------------------------------------
