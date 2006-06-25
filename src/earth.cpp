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

#include "main.hpp"
#include "opengl.hpp"
#include "earth.hpp"

//-----------------------------------------------------------------------------

ent::earth::earth(float k) : free(data->get_obj("earth.obj")), dist(k)
{
}

void ent::earth::draw_fill(int flags)
{
    glPushMatrix();
    {
        glScalef(dist, dist, dist);
        obj_draw_file(file);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
