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

#include "opengl.hpp"
#include "mode.hpp"

//-----------------------------------------------------------------------------

bool mode::mode::point(const float[3], const float[3], int, int)
{
    return false;
}

bool mode::mode::click(int, bool)
{
    return false;
}

bool mode::mode::keybd(int, bool, int)
{
    return false;
}

bool mode::mode::timer(float dt)
{
    return false;
}

//-----------------------------------------------------------------------------

GLfloat mode::mode::view(const GLfloat *frustum)
{
    return world.view(true, frustum);
}

void mode::mode::draw()
{
    world.draw(true);
}

//-----------------------------------------------------------------------------
