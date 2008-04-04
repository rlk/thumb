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
    index(glob->load_texture("mipmap-test.png", GL_NEAREST)),
    cache(glob->load_texture("world.200408-07-00-00.png"))
{
}

uni::geomap::~geomap()
{
    glob->free_texture(cache);
    glob->free_texture(index);
}

//-----------------------------------------------------------------------------

void uni::geomap::draw()
{
    index->bind(GL_TEXTURE1);
    cache->bind(GL_TEXTURE2);
    {
        ogl::program::current->uniform("index", 1);
        ogl::program::current->uniform("cache", 2);

        ogl::program::current->uniform("data_size", 86400.0, 43200.0);
        ogl::program::current->uniform("page_size",   512.0,   512.0);

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
    cache->free(GL_TEXTURE2);
    index->free(GL_TEXTURE1);
}

//-----------------------------------------------------------------------------
