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
#include "app-glob.hpp"
#include "app-prog.hpp"
#include "app-event.hpp"
#include "app-frustum.hpp"
#include "dpy-channel.hpp"
#include "dpy-normal.hpp"

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

    chani = std::max(0, app::get_attr_d(node, "channel"));
}

dpy::normal::~normal()
{
    delete frust;
}

//-----------------------------------------------------------------------------

void dpy::normal::get_frustums(app::frustum_v& frustums)
{
    assert(frust);

    // Add my frustum to the list.

    frustums.push_back(frust);
}

void dpy::normal::prep(int chanc, dpy::channel **chanv)
{
    assert(frust);

    // Apply the channel view position to the frustum.

    if (chani < chanc)
        frust->calc_user_planes(chanv[chani]->get_p());
}

int dpy::normal::draw(int chanc, dpy::channel **chanv,
                      int frusi, app::frustum  *frusp)
{
    if (chani < chanc)
    {
        assert(chanv[chani]);
        assert(P);

        // Draw the scene to the off-screen buffer.

        chanv[chani]->bind();
        {
            ::prog->draw(frusi, frusp);
        }
        chanv[chani]->free();

        // Draw the off-screen buffer to the screen.

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        chanv[chani]->bind_color(GL_TEXTURE0);
        {
            P->bind();
            {
                fill(frust->get_w(),
                     frust->get_h(),
                     chanv[chani]->get_w(),
                     chanv[chani]->get_h());
            }
            P->free();
        }
        chanv[chani]->free_color(GL_TEXTURE0);
    }

    return 1;  // Return the total number of frusta.
}

int dpy::normal::test(int chanc, dpy::channel **chanv, int index)
{
    if (chani < chanc)
    {
        assert(chanv[chani]);
        assert(P);

        // Draw the calibration pattern to the off-screen buffer.

        chanv[chani]->bind();
        {
            chanv[chani]->test();
        }
        chanv[chani]->free();

        // Draw the off-screen buffer to the screen.

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        chanv[chani]->bind_color(GL_TEXTURE0);
        {
            P->bind();
            {
                fill(frust->get_w(),
                     frust->get_h(),
                     chanv[chani]->get_w(),
                     chanv[chani]->get_h());
            }
            P->free();
        }
        chanv[chani]->free_color(GL_TEXTURE0);
    }

    return 1;  // Return the total number of frusta.
}

//-----------------------------------------------------------------------------

bool dpy::normal::project_event(app::event *E, int x, int y)
{
    assert(frust);

    // Determine whether the pointer falls within the viewport.

    double X = double(x - viewport[0]) / double(viewport[2]);
    double Y = double(viewport[1] - y) / double(viewport[3]);

    if (0.0 <= X && X < 1.0 &&
        0.0 <= Y && Y < 1.0)

        // Let the frustum project the pointer into space.

        return frust->project_event(E, X, Y);
    else
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
    assert(frust);

    // Do the local startup or shutdown.

    switch (E->get_type())
    {
    case E_START: process_start(E); break;
    case E_CLOSE: process_close(E); break;
    }

    // Let the frustum handle the event.

    return frust->process_event(E);
}

//-----------------------------------------------------------------------------
