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
#include "ogl-program.hpp"
#include "dpy-channel.hpp"
#include "dpy-normal.hpp"

//-----------------------------------------------------------------------------

dpy::normal::normal(app::node node) : display(node), frust(0), P(0)
{
    app::node curr;

    // Check the display definition for a frustum, or create a default

    if ((curr = app::find(node, "frustum")))
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

int dpy::normal::get_frusc() const
{
    return 1;
}

void dpy::normal::get_frusv(app::frustum **frusv) const
{
    assert(frust);

    frusv[0] = frust;
}

//-----------------------------------------------------------------------------

void dpy::normal::prep(int chanc, const dpy::channel *const *chanv)
{
    assert(frust);

    // Apply the channel view position to the frustum.

    if (chani < chanc)
        frust->set_viewpoint(chanv[chani]->get_p());
}

void dpy::normal::draw(int chanc, const dpy::channel *const *chanv, int frusi)
{
    if (chani < chanc)
    {
        assert(chanv[chani]);
        assert(P);

        // Draw the scene to the off-screen buffer.

        chanv[chani]->bind();
        {
            ::prog->draw(frusi, frust);
        }
        chanv[chani]->free();
        chanv[chani]->proc();

        // Draw the off-screen buffer to the screen.

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
}

void dpy::normal::test(int chanc, const dpy::channel *const *chanv, int index)
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
}

//-----------------------------------------------------------------------------

bool dpy::normal::pointer_to_3D(app::event *E, int x, int y)
{
    assert(frust);

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frust->pointer_to_3D(E, x - viewport[0],
                         viewport[3] - y + viewport[1]);
    else
        return false;
}

bool dpy::normal::process_start(app::event *E)
{
    // Initialize the shader.

    P = ::glob->load_program("normal.xml");

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
