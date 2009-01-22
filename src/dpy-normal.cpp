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

#include "dpy-normal.hpp"
#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-user.hpp"
#include "app-prog.hpp"

//-----------------------------------------------------------------------------

dpy::normal::normal(app::node tile,
                    app::node node, const int *window) : frust(0), P(0)
{
    app::node curr;

    x = window[0];
    y = window[1];
    w = window[2];
    h = window[3];

    // Check the display definition for a frustum.

    if      ((curr = app::find(node, "frustum")))
        frust = new app::frustum(curr, w, h);

    // If none, check the tile definition for one.

    else if ((curr = app::find(tile, "frustum")))
        frust = new app::frustum(curr, w, h);

    // If still none, create a default.

    else
        frust = new app::frustum(0, w, h);

    // Note the view index.

    index = app::get_attr_d(node, "view");
}

dpy::normal::~normal()
{
    if (frust) delete frust;
}

//-----------------------------------------------------------------------------

bool dpy::normal::input_point(int i, const double *p, const double *q)
{
    return frust ? frust->input_point(i, p, q) : false;
}

bool dpy::normal::input_click(int i, int b, int m, bool d)
{
    return frust ? frust->input_click(i, b, m, d) : false;
}

bool dpy::normal::input_keybd(int c, int k, int m, bool d)
{
    return frust ? frust->input_keybd(c, k, m, d) : false;
}

//-----------------------------------------------------------------------------

bool dpy::normal::pick(double *p, double *q, int wx, int wy)
{
    if (frust)
    {
        double kx = double(wx - x) / w;
        double ky = double(wy - y) / h;

        if (0.0 <= kx && kx < 1.0 &&
            0.0 <= ky && ky < 1.0)
            frust->pick(p, q, kx, ky);

        return true;
    }
    return false;
}

void dpy::normal::prep(app::view_v& views, app::frustum_v& frusta)
{
    if (frust)
    {
        // Apply the viewpoint and view to my frustum.

        frust->calc_user_planes(views[index]->get_p());
        frust->calc_view_planes(::user->get_M(),
                                ::user->get_I());

        // Add my frustum to the list.

        frusta.push_back(frust);
    }

    // Ensure the draw shader is initialized.

    if (P == 0 && (P = ::glob->load_program("glsl/dpy/normal.vert",
                                            "glsl/dpy/normal.frag")))
    {
        P->bind();
        {
            P->uniform("map", 0);
        }
        P->free();
    }
}

void dpy::normal::draw(app::view_v& views, int &i, bool calibrate, bool me)
{
    if (views[index])
    {
        // Draw the scene to the off-screen buffer.

        views[index]->bind();
        {
            if (calibrate)
            {
                const GLubyte *c = views[index]->get_c();
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            else ::prog->draw(i++);
        }
        views[index]->free();

        // Draw the off-screen buffer to the screen.

        if (P)
        {
            const double kx = double(views[index]->get_w()) / w;
            const double ky = double(views[index]->get_h()) / h;

            glViewport(x, y, w, h);

            views[index]->bind_color(GL_TEXTURE0);
            P->bind();
            {
                P->uniform("frag_d", -x * kx, -y * ky);
                P->uniform("frag_k",      kx,      ky);

                glBegin(GL_QUADS);
                {
                    glVertex2f(-1.0f, -1.0f);
                    glVertex2f(+1.0f, -1.0f);
                    glVertex2f(+1.0f, +1.0f);
                    glVertex2f(-1.0f, +1.0f);
                }
                glEnd();
            }
            P->free();
            views[index]->free_color(GL_TEXTURE0);
        }
    }
}

//-----------------------------------------------------------------------------
