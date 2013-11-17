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
#include <dpy-oculus.hpp>

//-----------------------------------------------------------------------------

dpy::oculus::oculus(app::node p) :
    display(p), P(0)
{
}

dpy::oculus::~oculus()
{
}

//-----------------------------------------------------------------------------

int dpy::oculus::get_frusc() const
{
    return 0;
    return 2;
}

void dpy::oculus::get_frusv(app::frustum **frusv) const
{
    /*
    OVR::Util::Render::StereoEyeParams L;
    OVR::Util::Render::StereoEyeParams R;

    L = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    R = Stereo.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
    */
}

//-----------------------------------------------------------------------------

void dpy::oculus::prep(int chanc, const dpy::channel *const *chanv)
{
}

void dpy::oculus::draw(int chanc, const dpy::channel * const *chanv, int frusi)
{
#if 0
    if (chanc > 1)
    {
        assert(chanv[0]);
        assert(chanv[1]);
        assert(P);

        // Draw the scene to the off-screen buffer.

        chanv[0]->bind();
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::host->draw(frusi + 0, frustL, 0);
        }
        chanv[0]->free();
        chanv[1]->bind();
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
#endif
}

void dpy::oculus::test(int chanc, const dpy::channel *const *chanv, int index)
{
}

//-----------------------------------------------------------------------------

bool dpy::oculus::pointer_to_3D(app::event *E, int x, int y)
{
#if 0
    assert(frustL);

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frustL->pointer_to_3D(E, x - viewport[0],
                          viewport[3] - y + viewport[1]);
    else
        return false;
#endif
    return false;
}

bool dpy::oculus::process_start(app::event *E)
{
    // Initialize the shader.

    if ((P = ::glob->load_program("oculus.xml")))
    {
    }
    return false;
}

bool dpy::oculus::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(P);

    P = 0;

    return false;
}

bool dpy::oculus::process_event(app::event *E)
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
