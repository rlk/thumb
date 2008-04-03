//  Copyright (C) 2007 Robert Kooima
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

#include "geomap.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

uni::geomap::geomap() :
    index(glob->load_texture("mipmap.png"))
{
}

uni::geomap::~geomap()
{
    glob->free_texture(index);
}

//-----------------------------------------------------------------------------

void uni::geomap::draw()
{
    index->bind(GL_TEXTURE1);
    {
        ogl::program::current->uniform("size", 86400.0, 43200.0);

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glRecti(-1, -1, +1, +1);

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    index->free(GL_TEXTURE1);
}

//-----------------------------------------------------------------------------
