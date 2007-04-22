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
#include "imgdata.hpp"

//-----------------------------------------------------------------------------

static const GLenum format[] = {
    0,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RGB,
    GL_RGBA
};

//-----------------------------------------------------------------------------

ogl::imgdata::imgdata(GLsizei w, GLsizei h, GLsizei b) :
    p(new GLubyte[w * h * b]),
    w(w),
    h(h),
    b(b)
{
    init();
}

ogl::imgdata::~imgdata()
{
    fini();

    if (p) delete [] p;
}

//-----------------------------------------------------------------------------

void ogl::imgdata::blit(const GLubyte *P, GLsizei X, GLsizei Y,
                                          GLsizei W, GLsizei H)
{
    // Copy the given pixel data to the memory buffer.

    for (GLsizei R = 0; R < H; ++R)
    {
        for (GLsizei C = 0; C < W; ++C)
        {
            GLsizei r = R + X;
            GLsizei c = C + Y;

            for (GLsizei i = 0; i < b; ++i)
                p[(r * w + c) * b + i] = P[(R * W + C) * b + i];
        }
    }

    // Copy the given pixel data to the texture object.

    bind();
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, W, H, format[b],
                        GL_UNSIGNED_BYTE, P);
    }
    free();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::imgdata::bind() const
{
    glBindTexture(GL_TEXTURE_2D, object);
}

void ogl::imgdata::free() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

//-----------------------------------------------------------------------------

void ogl::imgdata::init()
{
    glGenTextures(1, &object);
    glBindTexture(GL_TEXTURE_2D, object);

    glTexImage2D(GL_TEXTURE_2D, 0, format[b], w, h, 0,
                 format[b], GL_UNSIGNED_BYTE, p);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    OGLCK();
}

void ogl::imgdata::fini()
{
    glDeleteTextures(1, &object);
    OGLCK();
}

//-----------------------------------------------------------------------------
