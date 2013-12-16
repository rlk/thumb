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

#include <etc-vector.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-normal.hpp>

//-----------------------------------------------------------------------------

dpy::normal::normal(app::node p) : display(p), frust(0), program(0)
{
    // Check the display definition for a frustum, or create a default

    if (app::node n = p.find("frustum"))
        frust = new app::calibrated_frustum(n);
    else
        frust = new app::calibrated_frustum();

    // Note the channel index.

    chani = std::max(0, p.get_i("channel"));
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
        frust->set_eye(chanv[chani]->get_eye());
}

void dpy::normal::draw(int chanc, const dpy::channel *const *chanv, int frusi)
{
    if (chani < chanc)
    {
        assert(chanv[chani]);
        assert(program);

        // Draw the scene to the off-screen buffer.

        chanv[chani]->bind();
        {
            ::host->draw(frusi, frust, chani);
        }
        chanv[chani]->free();
        chanv[chani]->proc();

        // Draw the off-screen buffer to the screen.

        chanv[chani]->bind_color(GL_TEXTURE0);
        {
            program->bind();
            {
                fill(frust->get_width(),
                     frust->get_height(),
                     chanv[chani]->get_width(),
                     chanv[chani]->get_height());
            }
            program->free();
        }
        chanv[chani]->free_color(GL_TEXTURE0);
    }
}

void dpy::normal::test(int chanc, const dpy::channel *const *chanv, int index)
{
    if (chani < chanc)
    {
        assert(chanv[chani]);
        assert(program);

        // Draw the calibration pattern to the off-screen buffer.

        chanv[chani]->bind();
        {
            chanv[chani]->test();
        }
        chanv[chani]->free();

        // Draw the off-screen buffer to the screen.

        chanv[chani]->bind_color(GL_TEXTURE0);
        {
            program->bind();
            {
                fill(frust->get_width(),
                     frust->get_height(),
                     chanv[chani]->get_width(),
                     chanv[chani]->get_height());
            }
            program->free();
        }
        chanv[chani]->free_color(GL_TEXTURE0);
    }
}

//-----------------------------------------------------------------------------

bool dpy::normal::pointer_to_3D(app::event *E, int x, int y)
{
    // Let the frustum project the pointer into space.

    assert(frust);

    double s = double(x - viewport[0]) / viewport[2];
    double t = double(y - viewport[1]) / viewport[3];

    if (0.0 <= s && s < 1.0 && 0.0 <= t && t < 1.0)
        return frust->pointer_to_3D(E, s, t);
    else
        return false;
}

bool dpy::normal::process_start(app::event *E)
{
    // Initialize the shader.

    program = ::glob->load_program("dpy/normal.xml");

    return false;
}

bool dpy::normal::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(program);

    program = 0;

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
