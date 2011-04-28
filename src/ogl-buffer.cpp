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

#include <ogl-buffer.hpp>

//-----------------------------------------------------------------------------

ogl::buffer::buffer(GLenum t, GLsizei s, GLenum p) : t(t), s(s), p(p)
{
    glGenBuffersARB(1, &o);

    bind();
    zero();
    free();

    OGLCK();
}

ogl::buffer::~buffer()
{
    glDeleteBuffersARB(1, &o);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::buffer::bind() const
{
    glBindBufferARB(t, o);
    OGLCK();
}

void ogl::buffer::bind(GLenum target) const
{
    glBindBufferARB(target, o);
    OGLCK();
}

void ogl::buffer::free() const
{
    glBindBufferARB(t, 0);
    OGLCK();
}

void ogl::buffer::free(GLenum target) const
{
    glBindBufferARB(target, 0);
    OGLCK();
}

void ogl::buffer::zero() const
{
    glBufferDataARB(t, s, 0, p);
    OGLCK();
}

//-----------------------------------------------------------------------------

void *ogl::buffer::rmap() const
{
    return glMapBufferARB(t, GL_READ_ONLY_ARB);
}

void *ogl::buffer::wmap() const
{
    return glMapBufferARB(t, GL_WRITE_ONLY_ARB);
}

void *ogl::buffer::amap() const
{
    return glMapBufferARB(t, GL_READ_WRITE_ARB);
}

void ogl::buffer::umap() const
{
    glUnmapBufferARB(t);
    OGLCK();
}

//-----------------------------------------------------------------------------

