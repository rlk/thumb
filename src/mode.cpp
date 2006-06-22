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
    ent::entity::phys_step(dt);
    return false;
}

//-----------------------------------------------------------------------------

void mode::mode::draw() const
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        // Draw the scene.

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);

        scene.draw(ent::flag_fill);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------
