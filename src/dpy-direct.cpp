
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
#include <cstdio>

#include <etc-math.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-direct.hpp>

//-----------------------------------------------------------------------------

dpy::direct::direct(app::node p) : display(p), frust(0)
{
    // Check the display definition for a frustum, or create a default

    if (app::node n = p.find("frustum"))
        frust = new app::frustum(n, viewport[2], viewport[3]);
    else
        frust = new app::frustum(0, viewport[2], viewport[3]);

    // Note the channel index.

    chani = std::max(0, p.get_i("channel"));
}

dpy::direct::~direct()
{
    delete frust;
}

//-----------------------------------------------------------------------------

int dpy::direct::get_frusc() const
{
    return 1;
}

void dpy::direct::get_frusv(app::frustum **frusv) const
{
    frusv[0] = frust;
}

//-----------------------------------------------------------------------------

void dpy::direct::prep(int chanc, const dpy::channel *const *chanv)
{
    if (chani < chanc)
        frust->set_viewpoint(chanv[chani]->get_p());
}

void dpy::direct::draw(int chanc, const dpy::channel *const *chanv, int frusi)
{
    glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
    {
        glScissor (viewport[0], viewport[1], viewport[2], viewport[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        glEnable(GL_SCISSOR_TEST);
        {
            ::host->draw(frusi, frust, chani);
        }
        glDisable(GL_SCISSOR_TEST);
    }
    glPopAttrib();
}

void dpy::direct::test(int chanc, const dpy::channel *const *chanv, int index)
{
    if (chani < chanc)
    {
        glPushAttrib(GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
        {
            glScissor (viewport[0], viewport[1], viewport[2], viewport[3]);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

            glEnable(GL_SCISSOR_TEST);
            {
                chanv[chani]->test();
            }
            glDisable(GL_SCISSOR_TEST);
        }
        glPopAttrib();
    }
}

//-----------------------------------------------------------------------------

bool dpy::direct::pointer_to_3D(app::event *E, int x, int y)
{
    assert(frust);

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frust->pointer_to_3D(E, x - viewport[0],
                                       y - viewport[1]);
    else
        return false;
}

//-----------------------------------------------------------------------------
