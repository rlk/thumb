//  Copyright (C) 2007-2011 Robert Kooima
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
    glGenBuffers(1, &o);

    bind();
    zero();
    free();
}

ogl::buffer::~buffer()
{
    glDeleteBuffers(1, &o);
}

//-----------------------------------------------------------------------------

void ogl::buffer::bind() const
{
    glBindBuffer(t, o);
}

void ogl::buffer::bind(GLenum target) const
{
    glBindBuffer(target, o);
}

void ogl::buffer::free() const
{
    glBindBuffer(t, 0);
}

void ogl::buffer::free(GLenum target) const
{
    glBindBuffer(target, 0);
}

void ogl::buffer::zero() const
{
    glBufferData(t, s, 0, p);
}

//-----------------------------------------------------------------------------

void *ogl::buffer::rmap() const
{
    return glMapBuffer(t, GL_READ_ONLY);
}

void *ogl::buffer::wmap() const
{
    return glMapBuffer(t, GL_WRITE_ONLY);
}

void *ogl::buffer::amap() const
{
    return glMapBuffer(t, GL_READ_WRITE);
}

void ogl::buffer::umap() const
{
    glUnmapBuffer(t);
}

//-----------------------------------------------------------------------------

