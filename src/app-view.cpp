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
#include "app-view.hpp"
#include "app-glob.hpp"

//-----------------------------------------------------------------------------

app::view::view(app::node node, const int *buffer) :
    w(buffer[0]),
    h(buffer[1]), back(0)
{
    v[0] = p[0] = get_attr_f(node, "x");
    v[1] = p[1] = get_attr_f(node, "y");
    v[2] = p[2] = get_attr_f(node, "z");

    c[0] = GLubyte(get_attr_f(node, "r", 1.0) * 0xFF);
    c[1] = GLubyte(get_attr_f(node, "g", 1.0) * 0xFF);
    c[2] = GLubyte(get_attr_f(node, "b", 1.0) * 0xFF);
    c[3] = GLubyte(get_attr_f(node, "a", 1.0) * 0xFF);
}

app::view::~view()
{
//  if (back) ::glob->free_frame(back); HACK
}

void app::view::set_head(const double *p,
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

void app::view::bind()
{
    // Ensure the off-screen buffer exists.  Bind it as render target.

    if (back == 0)
        back = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                       GL_RGBA8, true, false);
    if (back)
        back->bind();
}

void app::view::free()
{
    // Unbind the off-screen render target.

    if (back)
        back->free();
}

/*
void app::view::draw(const int *rect, bool focus)
{
    double frag_d[2] = { 0, 0 };
    double frag_k[2] = { 1, 1 };

    // Set the view position.

    ::user->set_P(P);

    // If we are rendering directly on-screen...

    if (::user->get_type() == user::type_mono)
    {
        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(rect[0]);
        frag_d[1] =            -double(rect[1]);
        frag_k[0] = double(w) / double(rect[2]);
        frag_k[1] = double(h) / double(rect[3]);

        // Draw the scene.

        glViewport(rect[0], rect[1], rect[2], rect[3]);

        ::prog->draw(frag_d, frag_k);
    }
    else
    {
        // Make sure the off-screen buffer exists.

        if (back == 0)
            back = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                           GL_RGBA8, true, false);
        if (back)
        {
            back->bind();
            {
                if (::user->get_mode() == user::mode_norm)
                {
                    // Draw the scene normally.

                    glClear(GL_COLOR_BUFFER_BIT);
                    ::prog->draw(frag_d, frag_k);
                }
                if (::user->get_mode() == user::mode_test)
                {
                    float r = color[0] * (focus ? 1.0f : 0.5f);
                    float g = color[1] * (focus ? 1.0f : 0.5f);
                    float b = color[2] * (focus ? 1.0f : 0.5f);

                    // Draw the test pattern.

                    glPushAttrib(GL_COLOR_BUFFER_BIT);
                    glClearColor(r, g, b, 1);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glPopAttrib();
                }
            }
            back->free();
        }
    }
}
*/

//-----------------------------------------------------------------------------
