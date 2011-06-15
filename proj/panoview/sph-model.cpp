//  Copyright (C) 2005-2011 Robert Kooima
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

#include <ogl-opengl.hpp>

#include "sph-model.hpp"

//------------------------------------------------------------------------------

sph_model::sph_model()
{
}

sph_model::~sph_model()
{
}

//------------------------------------------------------------------------------

void sph_model::draw(int buffer_w, int buffer_h)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(0);

    glBegin(GL_QUADS);
    {
        glVertex3f(-10, -10, -10);
        glVertex3f( 10, -10, -10);
        glVertex3f( 10,  10, -10);
        glVertex3f(-10,  10, -10);
    }
    glEnd();
}

//------------------------------------------------------------------------------
