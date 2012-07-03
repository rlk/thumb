//  Copyright (C) 2007-2011 Robert Kooima
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

#include <etc-math.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-fulldome.hpp>

//-----------------------------------------------------------------------------

dpy::fulldome::fulldome(app::node p) : display(p), P(0)
{
    app::node f;

    for (f = p.find("frustum"); f; f = p.next(f, "frustum"))
    {
        frusta.push_back(new app::frustum(f, viewport[2] / 2, viewport[3] / 2));
    }
}

dpy::fulldome::~fulldome()
{
    // TODO: delete frusta
}

//-----------------------------------------------------------------------------

// Return the number of frusta.

int dpy::fulldome::get_frusc() const
{
    return int(frusta.size());
}

// Return the frustum pointers.

void dpy::fulldome::get_frusv(app::frustum **frusv) const
{
    for (int i = 0; i < get_frusc(); ++i)
        frusv[i] = frusta[i];
}

//-----------------------------------------------------------------------------

void dpy::fulldome::prep(int chanc, const dpy::channel *const *chanv)
{
    // Apply the channel view positions to the frusta.

    for (int i = 0; i < chanc && i < get_frusc(); ++i)
        frusta[i]->set_viewpoint(chanv[i]->get_p());
}

void dpy::fulldome::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    assert(P);

    int i;

    // Draw the scene to the off-screen buffer.

    for (i = 0; i < chanc && i < get_frusc(); ++i)
    {
        chanv[i]->bind();
        {
            ::host->draw(frusi + i, frusta[i], i);
        }
        chanv[i]->free();
    }

    // Draw the off-screen buffer to the screen.

    for (i = 0; i < chanc && i < get_frusc(); ++i)
        chanv[i]->bind_color(GL_TEXTURE0 + i);

    if (chanc)
    {
        P->bind();
        {
            fill(viewport[2],
                 viewport[3],
                 chanv[0]->get_w(),
                 chanv[0]->get_h());
        }
        P->free();
    }

    for (i = 0; i < chanc && i < get_frusc(); ++i)
        chanv[i]->free_color(GL_TEXTURE0 + i);
}

void dpy::fulldome::test(int chanc, const dpy::channel *const *chanv, int index)
{
    assert(P);

    int i;

    // Draw the scene to the off-screen buffer.

    for (i = 0; i < chanc && i < get_frusc(); ++i)
        chanv[i]->test();

    // Draw the off-screen buffer to the screen.

    for (i = 0; i < chanc && i < get_frusc(); ++i)
        chanv[i]->bind_color(GL_TEXTURE0 + i);

    if (chanc)
    {
        P->bind();
        {
            fill(viewport[2],
                 viewport[3],
                 chanv[0]->get_w(),
                 chanv[0]->get_h());
        }
        P->free();
    }

    for (i = 0; i < chanc && i < get_frusc(); ++i)
        chanv[i]->free_color(GL_TEXTURE0 + i);
}

//-----------------------------------------------------------------------------

bool dpy::fulldome::pointer_to_3D(app::event *E, int x, int y)
{
    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frusta[0]->pointer_to_3D(E, x - viewport[0],
                             viewport[3] - y + viewport[1]);
    else
        return false;
}

bool dpy::fulldome::process_start(app::event *E)
{
    // Initialize the shader.

    if ((P = ::glob->load_program("fulldome.xml")))
    {
    }

    return false;
}

bool dpy::fulldome::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(P);

    P = 0;

    return false;
}

bool dpy::fulldome::process_event(app::event *E)
{
    // Do the local startup or shutdown.

    switch (E->get_type())
    {
    case E_START: process_start(E); break;
    case E_CLOSE: process_close(E); break;
    }
    return false;
}

//-----------------------------------------------------------------------------
