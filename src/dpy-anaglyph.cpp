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

#include <SDL_keyboard.h>

#include "util.hpp"
#include "matrix.hpp"
#include "app-user.hpp"
#include "app-glob.hpp"
#include "app-prog.hpp"
#include "ogl-opengl.hpp"
#include "dpy-anaglyph.hpp"

//-----------------------------------------------------------------------------

dpy::anaglyph::anaglyph(app::node tile,
                        app::node node, const int *window) :
    frustL(0),
    frustR(0),
    P(0),
    node(node)
{
    app::node curr;

    count = 0;

    x = window[0];
    y = window[1];
    w = window[2];
    h = window[3];

    // Check the display definition for a frustum.

    if      ((curr = app::find(node, "frustum")))
    {
        frustL = new app::frustum(curr, w, h);
        frustR = new app::frustum(curr, w, h);
    }

    // If none, check the tile definition for one.

    else if ((curr = app::find(tile, "frustum")))
    {
        frustL = new app::frustum(curr, w, h);
        frustR = new app::frustum(curr, w, h);
    }

    // If still none, create a default.

    else
    {
        frustL = new app::frustum(0, w, h);
        frustR = new app::frustum(0, w, h);
    }
}

dpy::anaglyph::~anaglyph()
{
    if (frustR) delete frustR;
    if (frustL) delete frustL;
}

//-----------------------------------------------------------------------------

bool dpy::anaglyph::pick(double *p, double *q, int wx, int wy)
{
    if (frustL)
    {
        double kx = double(wx - x) / w;
        double ky = double(wy - y) / h;

        if (0.0 <= kx && kx < 1.0 &&
            0.0 <= ky && ky < 1.0)
            frustL->pick(p, q, kx, ky);

        return true;
    }
    return false;
}

void dpy::anaglyph::prep(app::view_v& views, app::frustum_v& frusta)
{
    // Apply the viewpoint and view to my frusta.  Add the to the list.

    if (frustL)
    {
        frustL->calc_user_planes(views[0]->get_p());
        frustL->calc_view_planes(::user->get_M(),
                                 ::user->get_I());
        frusta.push_back(frustL);
    }
    if (frustR)
    {
        frustR->calc_user_planes(views[1]->get_p());
        frustR->calc_view_planes(::user->get_M(),
                                 ::user->get_I());
        frusta.push_back(frustR);
    }


    // Ensure the draw shader is initialized.

    if (P == 0 && (P = ::glob->load_program("glsl/dpy/anaglyph.vert",
                                            "glsl/dpy/anaglyph.frag")))
    {
        P->bind();
        {
            P->uniform("L_map", 0);
            P->uniform("R_map", 1);
        }
        P->free();
    }
}

//-----------------------------------------------------------------------------

void dpy::anaglyph::draw(app::view_v& views, int &i, bool calibrate, bool me)
{
    // Draw the scene to the off-screen buffers.

    if (views[0])
    {
        views[0]->bind();
        {
            if (calibrate)
            {
                const GLubyte *c = views[0]->get_c();
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
                i++;
            }
            else ::prog->draw(i++);
        }
        views[0]->free();
    }

    if (views[1])
    {
        views[1]->bind();
        {
            if (calibrate)
            {
                const GLubyte *c = views[1]->get_c();
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
                i++;
            }
            else ::prog->draw(i++);
        }
        views[1]->free();
    }

    count++;

    if (P)
    {
        // Combine the off-screen buffers to the screen.

        const double kx = double(views[0]->get_w()) / w;
        const double ky = double(views[0]->get_h()) / h;

        glViewport(x, y, w, h);

        views[0]->bind_color(GL_TEXTURE0);
        views[1]->bind_color(GL_TEXTURE1);
        P->bind();
        {
            double W = frustL->get_w();
            double H = frustL->get_h();

            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
                glOrtho(-W / 2, +W / 2, -H / 2, +H / 2, -1, +1);
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPushMatrix();
                glLoadIdentity();
            }

            P->uniform("frag_d", -x * kx, -y * ky);
            P->uniform("frag_k",      kx,      ky);
            P->uniform("luma", 0.30, 0.59, 0.11, 0.00);

            glBegin(GL_QUADS);
            {
                glVertex2f(-W / 2, -H / 2);
                glVertex2f(+W / 2, -H / 2);
                glVertex2f(+W / 2, +H / 2);
                glVertex2f(-W / 2, +H / 2);
            }
            glEnd();

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }
        }
        P->free();
        views[0]->bind_color(GL_TEXTURE0);
        views[1]->bind_color(GL_TEXTURE1);
    }
}

//-----------------------------------------------------------------------------
