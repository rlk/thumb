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

#include "ogl-lut.hpp"

//-----------------------------------------------------------------------------

ogl::lut::lut(GLsizei n, GLenum t, GLenum f, GLenum e) : n(n), target(t)
{
    glGenTextures(1, &texture);
    glBindTexture(t,  texture);

    glTexImage1D(t, 0, f, n, 0, e, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    OGLCK();
}

ogl::lut::~lut()
{
    glDeleteTextures(1, &texture);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::lut::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, texture);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

void ogl::lut::free(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

//-----------------------------------------------------------------------------

