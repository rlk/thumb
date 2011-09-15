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
#include <dpy-interlace.hpp>

//-----------------------------------------------------------------------------

dpy::interlace::interlace(app::node p) :
    display(p), frustL(0), frustR(0), P(0)
{
    // Check the display definition for a frustum, or create a default

    if (app::node n = p.find("frustum"))
    {
        frustL = new app::frustum(n, viewport[2], viewport[3]);
        frustR = new app::frustum(n, viewport[2], viewport[3]);
    }
    else
    {
        frustL = new app::frustum(0, viewport[2], viewport[3]);
        frustR = new app::frustum(0, viewport[2], viewport[3]);
    }
}

dpy::interlace::~interlace()
{
    delete frustR;
    delete frustL;
}

//-----------------------------------------------------------------------------

int dpy::interlace::get_frusc() const
{
    return 2;
}

void dpy::interlace::get_frusv(app::frustum **frusv) const
{
    assert(frustL);
    assert(frustR);

    frusv[0] = frustL;
    frusv[1] = frustR;
}

//-----------------------------------------------------------------------------

void dpy::interlace::prep(int chanc, const dpy::channel *const *chanv)
{
    assert(frustL);
    assert(frustR);

    // Apply the channel view positions to the frustums.

    if (chanc > 0) frustL->set_viewpoint(chanv[0]->get_p());
    if (chanc > 1) frustR->set_viewpoint(chanv[1]->get_p());
}

void dpy::interlace::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
    if (chanc > 1)
    {
        assert(chanv[0]);
        assert(chanv[1]);
        assert(P);

        // Draw the scene to the off-screen buffer.

        chanv[0]->bind();
        {
            ::host->draw(frusi + 0, frustL, 0);
        }
        chanv[0]->free();
        chanv[1]->bind();
        {
            ::host->draw(frusi + 1, frustR, 1);
        }
        chanv[1]->free();

        // Draw the off-screen buffer to the screen.

        chanv[0]->bind_color(GL_TEXTURE0);
        chanv[1]->bind_color(GL_TEXTURE1);
        {
            P->bind();
            {
                fill(frustL->get_w(),
                     frustL->get_h(),
                     chanv[0]->get_w(),
                     chanv[0]->get_h());
            }
            P->free();
        }
        chanv[1]->free_color(GL_TEXTURE1);
        chanv[0]->free_color(GL_TEXTURE0);
    }
}

void dpy::interlace::test(int chanc, const dpy::channel *const *chanv, int index)
{
    if (chanc > 1)
    {
        assert(chanv[0]);
        assert(chanv[1]);
        assert(P);

        // Draw the scene to the off-screen buffer.

        chanv[0]->bind();
        {
            chanv[0]->test();
        }
        chanv[0]->free();

        chanv[1]->bind();
        {
            chanv[0]->test();
        }
        chanv[1]->free();

        // Draw the off-screen buffer to the screen.

        chanv[0]->bind_color(GL_TEXTURE0);
        chanv[1]->bind_color(GL_TEXTURE1);
        {
            P->bind();
            {
                fill(frustL->get_w(),
                     frustL->get_h(),
                     chanv[0]->get_w(),
                     chanv[0]->get_h());
            }
            P->free();
        }
        chanv[1]->free_color(GL_TEXTURE1);
        chanv[0]->free_color(GL_TEXTURE0);
    }
}

//-----------------------------------------------------------------------------

bool dpy::interlace::pointer_to_3D(app::event *E, int x, int y)
{
    assert(frustL);

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frustL->pointer_to_3D(E, x - viewport[0],
                          viewport[3] - y + viewport[1]);
    else
        return false;
}

bool dpy::interlace::process_start(app::event *E)
{
    // Initialize the shader.

    if ((P = ::glob->load_program("interlace.xml")))
    {
    }

    return false;
}

bool dpy::interlace::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(P);

    P = 0;

    return false;
}

bool dpy::interlace::process_event(app::event *E)
{
    assert(frustL);
    assert(frustR);

    // Do the local startup or shutdown.

    switch (E->get_type())
    {
    case E_START: process_start(E); break;
    case E_CLOSE: process_close(E); break;
    }

    // Let the frustum handle the event.

    return (frustL->process_event(E) |
            frustR->process_event(E));
}

//-----------------------------------------------------------------------------
