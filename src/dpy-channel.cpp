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

#include "matrix.hpp"
#include "default.hpp"
#include "app-glob.hpp"
#include "app-event.hpp"
#include "ogl-frame.hpp"
#include "dpy-channel.hpp"

//-----------------------------------------------------------------------------

dpy::channel::channel(app::node node) : buffer(0)
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

//-----------------------------------------------------------------------------

void dpy::channel::test() const
{
    // Clear the bound buffer buffer to the calibration color.

    glClearColor(c[0], c[1], c[2], c[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void dpy::channel::bind() const
{
    // Bind the off-screen render target.

    assert(buffer);
    buffer->bind();
}

void dpy::channel::free() const
{
    // Unbind the off-screen render target.

    assert(buffer);
    buffer->free();
}

void dpy::channel::bind_color(GLenum t) const
{
    // Bind the off-screen render target for reading.

    assert(buffer);
    buffer->bind_color(t);
}

void dpy::channel::free_color(GLenum t) const
{
    // Unbind the off-screen render target for reading.

    assert(buffer);
    buffer->free_color(t);
}

//-----------------------------------------------------------------------------

bool dpy::channel::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_START:

        // Initialize the off-screen render target.

        assert(buffer == 0);
        buffer = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                         GL_RGBA8, true, false);
        break;

    case E_CLOSE:

        // Finalize the off-screen render target.

        assert(buffer != 0);
        ::glob->free_frame(buffer);
        buffer = 0;

        break;
    }
    return false;
}

//-----------------------------------------------------------------------------
