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

#include "matrix.hpp"
#include "app-glob.hpp"
#include "uni-georen.hpp"


//-----------------------------------------------------------------------------

uni::rp2lut::rp2lut()
    : ogl::lut(16, GL_TEXTURE_1D, GL_LUMINANCE16_ALPHA16,
                   GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT)
{
    GLushort *p = new GLushort[32];
    GLsizei   i;
    GLsizei   n = 16;
    int       d = 1 << n;

    // Compute the recipricol power-of-two function normalized to 16 bits.

    for (i = 0; i < n; ++i, d /= 2)
    {
        p[i * 2 + 0] = GLushort(std::min(d,     0xFFFF));
        p[i * 2 + 1] = GLushort(std::min(d / 2, 0xFFFF));
    }

    // Copy the table to the texture.

    blit(p, 0, n);

    // This lookup table does not interpolate.

    bind();
    {
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    free();

    delete [] p;
}

//-----------------------------------------------------------------------------

uni::renbuf::renbuf(GLsizei w, GLsizei h, GLenum f, bool d, bool s) :
    ogl::frame(w, h, GL_TEXTURE_RECTANGLE_ARB, f, true, d, s)
{
}

uni::renbuf::~renbuf()
{
}

void uni::renbuf::init(GLfloat r, GLfloat g, GLfloat b) const
{
    ogl::frame::bind(false);

    glClearColor(r, g, b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT |
            GL_STENCIL_BUFFER_BIT);

    ogl::frame::free();
}

void uni::renbuf::bind() const
{
    // Bind the render target and shader.

    ogl::frame::bind(false);

//  draw->bind();
}

void uni::renbuf::free() const
{
    // Unbind the render target and shader.

//  draw->free();

    ogl::frame::free();
}

//-----------------------------------------------------------------------------

uni::cylbuf::cylbuf(GLsizei w, GLsizei h) :
    uni::renbuf(w, h, GL_RGBA32F_ARB, true, false),
//  uni::renbuf(w, h, GL_FLOAT_RGBA32_NV, true, false),
    draw(::glob->load_program("glsl/uni/drawcyl.vert",
                              "glsl/uni/drawcyl.frag"))
{
}

void uni::cylbuf::bind() const
{
    uni::renbuf::bind();
    draw->bind();
}

void uni::cylbuf::free() const
{
    draw->free();
    uni::renbuf::free();
}

//-----------------------------------------------------------------------------

uni::difbuf::difbuf(GLsizei w, GLsizei h, uni::cylbuf& cyl) :
    uni::renbuf(w, h, GL_RGBA8, false, false), cyl(cyl)
{
}

void uni::difbuf::bind() const
{
    uni::renbuf::bind();
    cyl.bind_color();
}

void uni::difbuf::free() const
{
    cyl.free_color();
    uni::renbuf::free();
}

//-----------------------------------------------------------------------------

uni::nrmbuf::nrmbuf(GLsizei w, GLsizei h, uni::cylbuf& cyl) :
    uni::renbuf(w, h, GL_RGBA8, false, false), cyl(cyl)
{
}

void uni::nrmbuf::bind() const
{
    uni::renbuf::bind();
    cyl.bind_color();
}

void uni::nrmbuf::free() const
{
    cyl.free_color();
    uni::renbuf::free();
}

//-----------------------------------------------------------------------------

uni::georen::georen(GLsizei w, GLsizei h) :
    w(w),
    h(h),
    _cyl(w, h),
    _dif(w, h, _cyl),
    _nrm(w, h, _cyl)
{
}

uni::georen::~georen()
{
}

void uni::georen::bind() const
{
    // Bind the diffuse and normal render targets as textures.

    _cyl.bind_color(GL_TEXTURE0);
    _dif.bind_color(GL_TEXTURE1);
    _nrm.bind_color(GL_TEXTURE2);
}

void uni::georen::free() const
{
    // Unbind the diffuse and normal render targets.

    _nrm.free_color(GL_TEXTURE2);
    _dif.free_color(GL_TEXTURE1);
    _cyl.free_color(GL_TEXTURE0);
}

//-----------------------------------------------------------------------------
