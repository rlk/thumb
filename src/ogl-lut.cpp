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

ogl::lut::lut(GLsizei w,
              GLenum T, GLenum fi, GLenum fe, GLenum t) :
    target(T), object(0), formint(fi), formext(fe), type(t), w(w)
{
    glGenTextures(1, &object);
    glBindTexture(T,  object);

    glTexImage1D(target, 0, fi, w, 0, fe, t, 0);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    OGLCK();
}

ogl::lut::~lut()
{
    glDeleteTextures(1, &object);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::lut::blit(const GLvoid *P, GLsizei X, GLsizei W) const
{
    bind();
    {
        glTexSubImage1D(target, 0, X, W, formext, type, P);
    }
    free();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::lut::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, object);
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

