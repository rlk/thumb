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

#include "opengl.hpp"
#include "image.hpp"

//-----------------------------------------------------------------------------

ogl::image::image(GLsizei w, GLsizei h, GLenum t, GLenum f) :
    target(t), format(f), p(0), w(w), h(h)
{
    init();
}

ogl::image::~image()
{
    fini();

    if (p) delete [] p;
}

//-----------------------------------------------------------------------------

void ogl::image::blit(const GLubyte *P, GLsizei X, GLsizei Y,
                                        GLsizei W, GLsizei H)
{
    bind();
    {
        glTexSubImage2D(target, 0, X, Y, W, H, GL_RGBA, GL_UNSIGNED_BYTE, P);
    }
    free();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::image::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, object);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::image::free(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------

void ogl::image::init()
{
    // Create a new texture object.

    glGenTextures(1,     &object);
    glBindTexture(target, object);

    glTexImage2D(target, 0, format, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, p);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // If we have a buffer, we must be reloading.  It's no longer needed.

    if (p)
    {
        delete [] p;
        p = 0;
    }

    OGLCK();
}

void ogl::image::fini()
{
    // Allocate and store a copy of the image.

    p = new GLubyte[w * h * 4];

    bind();
    {
        glGetTexImage(target, 0, GL_RGBA, GL_UNSIGNED_BYTE, p);
    }
    free();

    // Delete the texture object.

    glDeleteTextures(1, &object);
    OGLCK();
}

//-----------------------------------------------------------------------------
