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
#include <app-user.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-fulldome.hpp>

//-----------------------------------------------------------------------------

dpy::fulldome::fulldome(app::node p) : display(p), P(0)
{
    int w = viewport[2] / 2;
    int h = viewport[3] / 3;

    app::node f;

    for (f = p.find("frustum"); f; f = p.next(f, "frustum"))
        frusta.push_back(new app::frustum(f, w, h));
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ::host->draw(frusi + i, frusta[i], i);
        }
        chanv[i]->free();
    }

    // Draw the off-screen buffer to the screen.

    P->bind();
    {
        if (chanc > 0)
        {
            chanv[0]->bind_color(GL_TEXTURE0);
            P->uniform("image[0]", 0);
            P->uniform("size[0]",  chanv[0]->get_w(), chanv[0]->get_h());
            P->uniform("P[0]", frusta[0]->get_P(), false);
        }
        if (chanc > 1)
        {
            chanv[1]->bind_color(GL_TEXTURE1);
            P->uniform("image[1]", 1);
            P->uniform("size[1]",  chanv[1]->get_w(), chanv[1]->get_h());
            P->uniform("P[1]", frusta[1]->get_P(), false);
        }
        if (chanc > 2)
        {
            chanv[2]->bind_color(GL_TEXTURE2);
            P->uniform("image[2]", 2);
            P->uniform("size[2]",  chanv[2]->get_w(), chanv[2]->get_h());
            P->uniform("P[2]", frusta[2]->get_P(), false);
        }
        if (chanc > 3)
        {
            chanv[3]->bind_color(GL_TEXTURE3);
            P->uniform("image[3]", 3);
            P->uniform("size[3]",  chanv[3]->get_w(), chanv[3]->get_h());
            P->uniform("P[3]", frusta[3]->get_P(), false);
        }

        fill(viewport[2],
             viewport[3], 0, 0);

        for (i = 0; i < chanc; ++i)
            chanv[i]->free_color(GL_TEXTURE0 + i);
    }
    P->free();
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
                 viewport[3], 0, 0);
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
    {
        double dw = viewport[2] / 2.0;
        double dh = viewport[3] / 2.0;

        double dx =  (x - viewport[0] - dw) / dw;
        double dy = -(y - viewport[1] - dh) / dh;

        double r =  sqrt(dx * dx + dy * dy);
        double a = atan2(dy, dx);

        if (r < 1.0)
        {
            double q[4];
            double B[16];

            load_idt(B);

            B[ 8] = -cos(a) * sin(r * M_PI / 2.0);
            B[ 9] = -         cos(r * M_PI / 2.0);
            B[10] = -sin(a) * sin(r * M_PI / 2.0);

            normalize(B + 8);
            crossprod(B + 0, B + 4, B + 8);
            normalize(B + 0);
            crossprod(B + 4, B + 8, B + 0);
            normalize(B + 4);

            mat_to_quat(q, B);

            E->mk_point(0, ::user->get_M() + 12, q);

            return true;
        }
    }
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
