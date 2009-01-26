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

#include "dpy-normal.hpp"
#include "matrix.hpp"
#include "app-event.hpp"
#include "app-glob.hpp"
#include "app-user.hpp"
#include "app-prog.hpp"

//-----------------------------------------------------------------------------

dpy::normal::normal(app::node node) : display(node), frust(0), P(0)
{
    app::node curr;

    // Check the display definition for a frustum, or create a default

    if      ((curr = app::find(node, "frustum")))
        frust = new app::frustum(curr, viewport[2], viewport[3]);
    else
        frust = new app::frustum(0,    viewport[2], viewport[3]);

    // Note the channel index.

    chani = app::get_attr_d(node, "channel");
}

dpy::normal::~normal()
{
    delete frust;
}

//-----------------------------------------------------------------------------

void dpy::normal::get_frusta(app::frustum_v& frusta)
{
    // Add my frustum to the list.

    frusta.push_back(frust);
}

void dpy::normal::draw(app::view_v& views, app::frustum *frust)
{
    assert(P);

    if (views[view_i])
    {
        // Draw the scene to the off-screen buffer.

        views[view_i]->bind();
        {
            if (calibrate)
                views[view_i]->draw();
            else
                ::prog->draw(frust);
        }
        views[view_i]->free();

        // Draw the off-screen buffer to the screen.

        if (P)
        {
            const double kx = double(views[view_i]->get_w()) / w;
            const double ky = double(views[view_i]->get_h()) / h;

            glViewport(x, y, w, h);

            views[view_i]->bind_color(GL_TEXTURE0);
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
            views[view_i]->free_color(GL_TEXTURE0);
        }
    }
}

//-----------------------------------------------------------------------------

bool dpy::normal::project_event(app::event *E, int x, int y)
{
    // Determine whether the pointer falls within the viewport.

    if (frust)
    {
        double X = double(wx - x) / w;
        double Y = double(wy - y) / h;

        if (0.0 <= X && X < 1.0 &&
            0.0 <= Y && Y < 1.0)

            // Let the frustum project the pointer into space.

            return frust->project_event(E, X, Y))
    }
    return false;
}

bool dpy::normal::process_start(app::event *E)
{
    // Initialize the shader.

    if ((P = ::glob->load_program("glsl/dpy/normal.vert",
                                  "glsl/dpy/normal.frag")))
    {
        P->bind();
        {
            P->uniform("map", 0);
        }
        P->free();
    }
    return false;
}

bool dpy::normal::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(P);

    P = 0;

    return false;
}

bool dpy::normal::process_event(app::event *E)
{
    // Do the local startup or shutdown.

    switch (E.get_type())
    {
    case E_START: process_start(E);
    case E_CLOSE: process_close(E);
    }

    // Let the frustum handle the event.

    if (frust)
        return frust->process_event(app::event *E);
    else
        return false;
}

//-----------------------------------------------------------------------------
