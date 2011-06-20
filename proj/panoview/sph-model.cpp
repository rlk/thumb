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

#include <GL/glew.h>

#include "sph-model.hpp"

//------------------------------------------------------------------------------

sph_model::sph_model()
{
}

sph_model::~sph_model()
{
}

//------------------------------------------------------------------------------

static const double cube_v[8][3] = {
    {  1,  1,  1 },
    { -1,  1,  1 },
    {  1, -1,  1 },
    { -1, -1,  1 },
    {  1,  1, -1 },
    { -1,  1, -1 },
    {  1, -1, -1 },
    { -1, -1, -1 },
};

static const int cube_i[6][4] = {
    { 0, 4, 6, 2 },
    { 5, 1, 3, 7 },
    { 5, 4, 0, 1 },
    { 3, 2, 6, 7 },
    { 1, 0, 2, 3 },
    { 4, 5, 7, 6 },
};

void sph_model::draw(int buffer_w, int buffer_h)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(0);

    glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
    
    glBegin(GL_QUADS);
    {
        for (int i = 0; i < 6; ++i)
        {
            glVertex3dv(cube_v[cube_i[i][0]]);
            glVertex3dv(cube_v[cube_i[i][1]]);
            glVertex3dv(cube_v[cube_i[i][2]]);
            glVertex3dv(cube_v[cube_i[i][3]]);
        }
    }
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
    
    for (int i = 0; i < 6; ++i)
    {
        glBegin(GL_LINE_LOOP);
        {
            glVertex3dv(cube_v[cube_i[i][0]]);
            glVertex3dv(cube_v[cube_i[i][1]]);
            glVertex3dv(cube_v[cube_i[i][2]]);
            glVertex3dv(cube_v[cube_i[i][3]]);
        }
        glEnd();
    }
}

//------------------------------------------------------------------------------
