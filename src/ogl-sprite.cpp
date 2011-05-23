//  Copyright (C) 2010-2011 Robert Kooima
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

#include <app-conf.hpp>
#include <app-glob.hpp>
#include <ogl-binding.hpp>
#include <ogl-sprite.hpp>

//-----------------------------------------------------------------------------

ogl::sprite::sprite() : binding(::glob->load_binding("sprite", "sprite"))
{
}

ogl::sprite::~sprite()
{
    ::glob->free_binding(binding);
}

//-----------------------------------------------------------------------------

void ogl::sprite::bind() const
{
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);

    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    binding->bind(false);
}

void ogl::sprite::free() const
{
    glDepthMask(GL_TRUE);
    
    glDisable(GL_BLEND);
    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

//-----------------------------------------------------------------------------
