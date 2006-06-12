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
#include "light.hpp"

//-----------------------------------------------------------------------------

ent::light::light(GLenum n) : name(n)
{
    turn_world(-60.0f, 1.0f, 0.0f, 0.0f);
    turn_world( 30.0f, 0.0f, 1.0f, 0.0f);
}

void ent::light::draw_fill() const
{
    GLfloat P[4] = { 0.0, 0.0, 1.0, 0.0 };

    glPushMatrix();
    {
        mult_M();

        glEnable (name);
        glLightfv(name, GL_POSITION, P);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
