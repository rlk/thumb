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

#include <SDL_keyboard.h>

#include "varrier.hpp"
#include "matrix.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

app::varrier::varrier(app::node node) : node(node)
{
    // Initialize the linescreen from the XML node or by setting defaults.

    pitch = get_attr_f(node, "pitch", 200.0000);
    angle = get_attr_f(node, "angle",   7.0000);
    thick = get_attr_f(node, "thick",   0.0160);
    shift = get_attr_f(node, "shift",   0.0000);
    cycle = get_attr_f(node, "cycle",   0.8125);
}

//-----------------------------------------------------------------------------

bool app::varrier::input_keybd(int c, int k, int m, bool d)
{
    bool b = false;

    if (m & KMOD_CTRL)
    {
        // Calibrate linescreen parameters.

        if (m & KMOD_SHIFT)
        {
            if      (k == SDLK_LEFT)  { angle -= 0.01; b = true; }
            else if (k == SDLK_RIGHT) { angle += 0.01; b = true; }
            else if (k == SDLK_DOWN)  { pitch -= 0.01; b = true; }
            else if (k == SDLK_UP)    { pitch += 0.01; b = true; }
        }
        else
        {
            if      (k == SDLK_LEFT)  { shift -= 0.00005; b = true; }
            else if (k == SDLK_RIGHT) { shift += 0.00005; b = true; }
            else if (k == SDLK_DOWN)  { thick -= 0.00010; b = true; }
            else if (k == SDLK_UP)    { thick += 0.00010; b = true; }
        }
    }

    if (b)
    {
        // Update the values in the XML node.

        set_attr_f(node, "pitch", pitch);
        set_attr_f(node, "angle", angle);
        set_attr_f(node, "thick", thick);
        set_attr_f(node, "shift", shift);
        set_attr_f(node, "cycle", cycle);

        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

void app::varrier::draw() const
{
/*
    double R[3];
    double U[3];
    double N[3];
    double v[3];
    double w[3];

    double dx, dy;
    double pp, ss;

    // Compute the screen space basis.

    R[0] = BR[0] - BL[0];
    R[1] = BR[1] - BL[1];
    R[2] = BR[2] - BL[2];

    U[0] = TL[0] - BL[0];
    U[1] = TL[1] - BL[1];
    U[2] = TL[2] - BL[2];

    crossprod(N, R, U);

    normalize(R);
    normalize(U);
    normalize(N);

    // Find the vector from the center of the screen to the eye.

    v[0] = P[0] - (TL[0] + BR[0]) * 0.5;
    v[1] = P[1] - (TL[1] + BR[1]) * 0.5;
    v[2] = P[2] - (TL[2] + BR[2]) * 0.5;

    // Transform this vector into screen space.

    w[0] = v[0] * R[0] + v[1] * R[1] + v[2] * R[2];
    w[1] = v[0] * U[0] + v[1] * U[1] + v[2] * U[2];
    w[2] = v[0] * N[0] + v[1] * N[1] + v[2] * N[2];

    // Compute the parallax due to optical thickness.

    dx = thick * w[0] / w[2];
    dy = thick * w[1] / w[2];

    // Compute the pitch and shift reduction due to optical thickness.

    pp = pitch * (w[2] - thick) / w[2];
    ss = shift * (w[2] - thick) / w[2];

    // Compose the line screen transformation matrix.

    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
        glScaled(pp, pp, 1);
        glRotated(-angle, 0, 0, 1);
        glTranslated(dx - ss, dy, 0);
    }
    glMatrixMode(GL_MODELVIEW);
*/
}

//-----------------------------------------------------------------------------
