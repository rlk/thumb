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

#include "georen.hpp"
#include "matrix.hpp"
#include "glob.hpp"


//-----------------------------------------------------------------------------

uni::renbuf::renbuf(GLsizei w, GLsizei h, GLenum f, bool d, bool s,
                    std::string name) :
    ogl::frame(w, h, GL_TEXTURE_RECTANGLE_ARB, f, d, s), draw(name)
{
    draw.uniform("cyl", 0);
    draw.uniform("src", 1);
}

uni::renbuf::~renbuf()
{
}

void uni::renbuf::init(GLfloat r, GLfloat g, GLfloat b) const
{
    ogl::frame::bind(false);
    glClearColor(r, g, b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ogl::frame::free();
}

void uni::renbuf::bind(int type) const
{
    // Bind the render target and shader.

    ogl::frame::bind(false);

    draw.bind(type);
}

void uni::renbuf::free(int type) const
{
    // Unbind the render target and shader.

    draw.free(type);

    ogl::frame::free();
}

//-----------------------------------------------------------------------------

uni::cylbuf::cylbuf(GLsizei w, GLsizei h) :
    uni::renbuf(w, h, GL_FLOAT_RGBA32_NV, true, false, "glsl/drawcyl")
{
}

//-----------------------------------------------------------------------------

uni::difbuf::difbuf(GLsizei w, GLsizei h, uni::cylbuf& cyl) :
    uni::renbuf(w, h, GL_RGBA8, false, false, "glsl/drawdif"), cyl(cyl)
{
}

void uni::difbuf::bind(int type) const
{
    uni::renbuf::bind(type);
    cyl.bind_color();
}

void uni::difbuf::free(int type) const
{
    cyl.free_color();
    uni::renbuf::free(type);
}

//-----------------------------------------------------------------------------

uni::nrmbuf::nrmbuf(GLsizei w, GLsizei h, uni::cylbuf& cyl) :
    uni::renbuf(w, h, GL_RGBA8, false, false, "glsl/drawnrm"), cyl(cyl)
{
}

void uni::nrmbuf::axis(const double *a) const
{
}

void uni::nrmbuf::bind(int type) const
{
    uni::renbuf::bind(type);
    cyl.bind_color();
}

void uni::nrmbuf::free(int type) const
{
    cyl.free_color();
    uni::renbuf::free(type);
}

//-----------------------------------------------------------------------------

uni::georen::georen(GLsizei w, GLsizei h) :
    w(w),
    h(h),
    draw(0),
//    draw(glob->load_program("glsl/drawlit.vert",
//                            "glsl/drawlit.frag")),
    _cyl(w, h),
    _dif(w, h, _cyl),
    _nrm(w, h, _cyl)
{
/*
    draw->bind();
    {
        draw->uniform("cyl", 0);
        draw->uniform("dif", 1);
        draw->uniform("nrm", 2);
    }
    draw->free();
*/
}

uni::georen::~georen()
{
//   glob->free_program(draw);
}

void uni::georen::bind() const
{
    // Bind the deferred illumination shader.

//  draw->bind();

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

    // Unbind the deferred illumination shader.

//  draw->free();
}

//-----------------------------------------------------------------------------
