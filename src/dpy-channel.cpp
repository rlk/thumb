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

#include <cassert>

#include "app-glob.hpp"
#include "matrix.hpp"
#include "default.hpp"
#include "dpy-channel.hpp"

//-----------------------------------------------------------------------------

dpy::channel::channel(app::node node) : back(0)
{
    v[0] = p[0] = app::get_attr_f(node, "x");
    v[1] = p[1] = app::get_attr_f(node, "y");
    v[2] = p[2] = app::get_attr_f(node, "z");

    c[0] = GLubyte(app::get_attr_f(node, "r", 1.0) * 0xFF);
    c[1] = GLubyte(app::get_attr_f(node, "g", 1.0) * 0xFF);
    c[2] = GLubyte(app::get_attr_f(node, "b", 1.0) * 0xFF);
    c[3] = GLubyte(app::get_attr_f(node, "a", 1.0) * 0xFF);

    w = app::get_attr_d(node, "w", DEFAULT_PIXEL_WIDTH);
    h = app::get_attr_d(node, "h", DEFAULT_PIXEL_HEIGHT);
}

dpy::channel::~channel()
{
}

void dpy::channel::set_head(const double *p,
                            const double *q)
{
    // Cache the view position in the head's coordinate system.

    double M[16], w[3];

    set_quaternion(M, q);

    mult_mat_vec3(w, M, v);

    this->p[0] = p[0] + w[0];
    this->p[1] = p[1] + w[1];
    this->p[2] = p[2] + w[2];
}

void dpy::channel::init()
{
    // Initialize the off-screen render target.

    assert(back == 0);
    back = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                   GL_RGBA8, true, false);
}

void dpy::channel::fini()
{
    // Finalize the off-screen render target.

    assert(back);
    ::glob->free_frame(back);
    back = 0;
}

void dpy::channel::bind() const
{
    // Bind the off-screen render target.

    assert(back);
    back->bind();
}

void dpy::channel::free() const
{
    // Unbind the off-screen render target.

    assert(back);
    back->free();
}

void dpy::channel::test() const
{
    // Clear the bound back buffer to the calibration color.

    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
