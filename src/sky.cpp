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
#include "main.hpp"
#include "sky.hpp"

//-----------------------------------------------------------------------------

ent::sky::sky(float k) :

    free(glob->load_geodata("sky.obj")),
    glow(glob->load_texture("sky_glow.png")),
    fill(glob->load_texture("sky_fill.png")),
    prog(glob->load_program("sky")),

    dist(k)
{
}

void ent::sky::draw_fill(int flags)
{
    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glScalef(dist, dist, dist);

        fill->bind(GL_TEXTURE0);
        glow->bind(GL_TEXTURE1);
        prog->bind();

        prog->uniform("fill", 0);
        prog->uniform("glow", 1);

        geometry->draw();

        prog->free();
        fill->free(GL_TEXTURE0);
        glow->free(GL_TEXTURE1);
    }
    glPopMatrix();
    glPopAttrib();
}

//-----------------------------------------------------------------------------
