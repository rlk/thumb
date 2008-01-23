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

#include "normal.hpp"
#include "glob.hpp"
#include "user.hpp"
#include "prog.hpp"

//-----------------------------------------------------------------------------

app::normal::normal(app::node tile,
                    app::node node, const int *window) : frust(0), P(0)
{
    app::node curr;

    x = window[0];
    y = window[1];
    w = window[2];
    h = window[3];

    // Check the display definition for a frustum.

    if      ((curr = find(node, "frustum")))
        frust = new app::frustum(curr);

    // If none, check the tile definition for one.

    else if ((curr = find(tile, "frustum")))
        frust = new app::frustum(curr);

    // Note the view index.

    index = get_attr_d(node, "view");
}

app::normal::~normal()
{
    if (frust) delete frust;
}

//-----------------------------------------------------------------------------

void app::normal::prep(view_v& views, frustum_v& frusta)
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

    if (P == 0 && (P = ::glob->load_program("glsl/normal.vert",
                                            "glsl/normal.frag")))
    {
        P->bind();
        {
            P->uniform("map", 0);
        }
        P->free();
    }
}

void app::normal::draw(view_v& views, int &i, bool calibrate)
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
