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

#include <SDL_keyboard.h>

#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-prog.hpp"
#include "app-event.hpp"
#include "app-frustum.hpp"
#include "ogl-program.hpp"
#include "dpy-channel.hpp"
#include "dpy-lenticular.hpp"

//-----------------------------------------------------------------------------

dpy::lenticular::lenticular(app::node node) :
    display(node),

    channels(app::get_attr_d(node, "channels", 1)),

    pitch(100.0),
    angle(  0.0),
    thick(  0.1),
    shift(  0.0),
    debug(  1.0),
    quality(1.0),

    P(0), node(node)
{
    app::node curr;

    int i;

    // Find a frustum definition, or create a default.  Clone the frustum and
    // create a default slice for each channel.

    for (i = 0; i < channels; ++i)
        if ((curr = app::find(node, "frustum")))
        {
            frust.push_back(new app::frustum(curr, viewport[2], viewport[3]));
            slice.push_back(slice_param(0.85));
        }
        else
        {
            frust.push_back(new app::frustum(0,    viewport[2], viewport[3]));
            slice.push_back(slice_param(0.85));
        }

    // Configure the array.

    if ((array = app::find(node, "array")))
    {
        pitch   = app::get_attr_f(array, "pitch", 100.0);
        angle   = app::get_attr_f(array, "angle",   0.0);
        thick   = app::get_attr_f(array, "thick",   0.1);
        shift   = app::get_attr_f(array, "shift",   0.0);
        quality = app::get_attr_f(array, "quality", 1.0);
    }

    // Configure the slices.

    for (curr = app::find(node,       "slice"); curr;
         curr = app::next(node, curr, "slice"))

        if ((i = app::get_attr_d(curr, "index")) < int(slice.size()))
        {
            slice[i].cycle = app::get_attr_f(curr, "cycle");
            slice[i].step0 = app::get_attr_f(curr, "step0");
            slice[i].step1 = app::get_attr_f(curr, "step1");
            slice[i].step2 = app::get_attr_f(curr, "step2");
            slice[i].step3 = app::get_attr_f(curr, "step3");
            slice[i].depth = app::get_attr_f(curr, "depth");
        }
}

dpy::lenticular::~lenticular()
{
    assert(!frust.empty());

    std::vector<app::frustum *>::iterator i;

    for (int i = 0; i < channels; ++i)
        delete frust[i];
}

//-----------------------------------------------------------------------------

void dpy::lenticular::get_frustums(app::frustum_v& frustums)
{
    assert(!frust.empty());

    // Add my frustums to the list.

    std::vector<app::frustum *>::iterator i;

    for (i = frust.begin(); i != frust.end(); ++i)
        frustums.push_back(*i);
}

void dpy::lenticular::prep(int chanc, dpy::channel **chanv)
{
    assert(!frust.empty());

    // Apply the channel view positions to the frustums.

    std::vector<app::frustum *>::iterator i;

    int c = 0;

    for (i = frust.begin(); i != frust.end() && c < chanc; ++i, ++c)
    {
        const double *p = chanv[c]->get_p();

        double q[3];

        q[0] = p[0] * debug;
        q[1] = p[1] * debug;
        q[2] = p[2] * debug;

        (*i)->calc_user_planes(chanv[c]->get_p());
    }
}

int dpy::lenticular::draw(int chanc, dpy::channel **chanv,
                          int frusi, app::frustum **frusv)
{
    int c;

    // Draw the scene to the off-screen buffers.

    for (c = 0; c < chanc; ++c)
    {
        chanv[c]->bind();
        ::prog->draw(frusi + c, frusv[frusi + c]);
        chanv[c]->free();
    }

    // Draw the off-screen buffers to the screen.

    for (c = 0; c < chanc; ++c)
        chanv[c]->bind_color(GL_TEXTURE0 + c);

    P->bind();
    {
        apply_uniforms();

        glViewport(viewport[0], viewport[1],
                   viewport[2], viewport[3]);
        glBegin(GL_QUADS);
        {
            glVertex2i(-1, -1);
            glVertex2i(+1, -1);
            glVertex2i(+1, +1);
            glVertex2i(-1, +1);
        }
        glEnd();
    }
    P->free();

    return channels;
}

int dpy::lenticular::test(int chanc, dpy::channel **chanv, int index)
{
    int c;

    // Draw the scene to the off-screen buffers.

    for (c = 0; c < chanc; ++c)
    {
        chanv[c]->bind();
        chanv[c]->test();
        chanv[c]->free();
    }

    // Draw the off-screen buffers to the screen.

    for (c = 0; c < chanc; ++c)
        chanv[c]->bind_color(GL_TEXTURE0 + c);

    P->bind();
    {
        apply_uniforms();

        glViewport(viewport[0], viewport[1],
                   viewport[2], viewport[3]);
        glBegin(GL_QUADS);
        {
            glVertex2i(-1, -1);
            glVertex2i(+1, -1);
            glVertex2i(+1, +1);
            glVertex2i(-1, +1);
        }
        glEnd();
    }
    P->free();

    return channels;
}

//-----------------------------------------------------------------------------

bool dpy::lenticular::pointer_to_3D(app::event *E, int x, int y)
{
    assert(!frust.empty());

    // Determine whether the pointer falls within the viewport.

    if (viewport[0] <= x && x < viewport[0] + viewport[2] &&
        viewport[1] <= y && y < viewport[1] + viewport[3])

        // Let the frustum project the pointer into space.

        return frust[0]->pointer_to_3D(E, x - viewport[0],
                            viewport[3] - y + viewport[1]);
    else
        return false;
}

bool dpy::lenticular::process_start(app::event *E)
{
    // Initialize the shader.

    if ((P = ::glob->load_program("lenticular.xml")))
    {
    }

    return false;
}

bool dpy::lenticular::process_close(app::event *E)
{
    // Finalize the shader.

    ::glob->free_program(P);

    P = 0;

    return false;
}

bool dpy::lenticular::process_keybd(app::event *E)
{
    const bool d = E->data.keybd.d;
    const int  k = E->data.keybd.k;
    const int  m = E->data.keybd.m;

    bool b = false;

    if (d && m & KMOD_CTRL)
    {
        if (m & KMOD_SHIFT)
        {
            if      (k == SDLK_LEFT)  { angle -= 0.00500; b = true; }
            else if (k == SDLK_RIGHT) { angle += 0.00500; b = true; }
            else if (k == SDLK_DOWN)  { pitch -= 0.01000; b = true; }
            else if (k == SDLK_UP)    { pitch += 0.01000; b = true; }
        }
        else
        {
            if (m & KMOD_CAPS)
            {
                if      (k == SDLK_LEFT)  { shift -= 0.00010; b = true; }
                else if (k == SDLK_RIGHT) { shift += 0.00010; b = true; }
                else if (k == SDLK_DOWN)  { thick += 0.00100; b = true; }
                else if (k == SDLK_UP)    { thick -= 0.00100; b = true; }
            }
            else
            {
                if      (k == SDLK_LEFT)  { shift -= 0.00001; b = true; }
                else if (k == SDLK_RIGHT) { shift += 0.00001; b = true; }
                else if (k == SDLK_DOWN)  { thick += 0.00010; b = true; }
                else if (k == SDLK_UP)    { thick -= 0.00010; b = true; }
            }
        }
    }

    if (b)
    {
        // Update the values in the XML node.

        app::set_attr_f(array, "pitch", pitch);
        app::set_attr_f(array, "angle", angle);
        app::set_attr_f(array, "thick", thick);
        app::set_attr_f(array, "shift", shift);

        return true;
    }

    return false;
}

bool dpy::lenticular::process_event(app::event *E)
{
    assert(E);

    std::vector<app::frustum *>::iterator i;

    // Do the local startup or shutdown.

    bool R = false;

    switch (E->get_type())
    {
    case E_KEYBD: R |= process_keybd(E); break;
    case E_START: R |= process_start(E); break;
    case E_CLOSE: R |= process_close(E); break;
    }

    // Let the frustums handle the event.

    if (R == false)
        for (i = frust.begin(); i != frust.end(); ++i)
            R |= (*i)->process_event(E);

    return R;
}

//-----------------------------------------------------------------------------

void dpy::lenticular::calc_transform(const double *u, double *v) const
{
    // Compute the debug-scaled pitch and optical thickness.

    const double p = pitch / debug;
    const double t = thick * debug;

    // Compute the pitch and shift reduction due to optical thickness.

    double pp =     p * (u[2] - t) / u[2];
    double ss = shift * (u[2] - t) / u[2];

    // Compute the parallax due to optical thickness.

    double dx = t * u[0] / u[2] - ss;
    double dy = t * u[1] / u[2];

    // Compose the line screen transformation matrix.

    double M[16];

    load_scl_mat(M, pp, pp,  1);
    Rmul_rot_mat(M,  0,  0,  1, -angle);
    Rmul_xlt_mat(M, dx, dy,  0);

    // We only need to transform X, so return the first row.

    v[0] = M[ 0];
    v[1] = M[ 4];
    v[2] = M[ 8];
    v[3] = M[12];
}

void dpy::lenticular::apply_uniforms() const
{
    static const std::string index[] = {
        "[0]",  "[1]",  "[2]",  "[3]",  "[4]",  "[5]",  "[6]",  "[7]",
        "[8]",  "[9]", "[10]", "[11]", "[12]", "[13]", "[14]", "[15]"
    };

    const double w = frust[0]->get_w();
    const double h = frust[0]->get_h();
    const double d = w / (3 * viewport[3]);

    P->uniform("size", w * 0.5, h * 0.5, 0.0, 1.0);
    P->uniform("eyes", channels);

    P->uniform("quality", quality);
    P->uniform("offset", -d, 0, d);
    P->uniform("corner", viewport[0], viewport[1]);

    for (int i = 0; i < channels; ++i)
    {
        const double e0 = 0.0;
        const double e6 = 1.0;
        const double e1 =      slice[i].step0;
        const double e5 =      slice[i].cycle;
        const double e2 = e1 + slice[i].step1;
        const double e4 = e5 - slice[i].step3;
        const double e3 = e4 - slice[i].step2;

        double v[4];

        calc_transform(frust[i]->get_disp_pos(), v);

        P->uniform("coeff" + index[i], v[0], v[1], v[2], v[3]);
        P->uniform("edge0" + index[i], e0, e0, e0);
        P->uniform("edge1" + index[i], e1, e1, e1);
        P->uniform("edge2" + index[i], e2, e2, e2);
        P->uniform("edge3" + index[i], e3, e3, e3);
        P->uniform("edge4" + index[i], e4, e4, e4);
        P->uniform("edge5" + index[i], e5, e5, e5);
        P->uniform("edge6" + index[i], e6, e6, e6);
        P->uniform("depth" + index[i], -slice[i].depth,
                                       -slice[i].depth,
                                       -slice[i].depth);
    }
}

//-----------------------------------------------------------------------------
