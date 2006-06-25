//  Copyright (C) 2005 Robert Kooima
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

#include <ode/ode.h>

#include "opengl.hpp"
#include "matrix.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

app::view::view(int w, int h, float n, float f, float z) :
    w(w), h(h), n(n), f(f), z(z)
{
    load_idt(M);

    move(0.0f, 3.0f, 5.0f);
}

//-----------------------------------------------------------------------------

void app::view::mult_S() const
{
    float T[16] = {
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f,
    };

    glMultMatrixf(T);
}

void app::view::mult_P() const
{
    float a = float(w) / float(h);

    // Apply the perspective projection.

    glFrustum(-a * z, +a * z, -z, +z, n, f);
}

void app::view::mult_O() const
{
    // Apply the orthogonal projection.

    glOrtho(0, w, h, 0, 0, 1);
}

void app::view::mult_M() const
{
    glMultMatrixf(M);
}

void app::view::mult_R() const
{
    float T[16];

    // Apply the current view rotation transform.

    T[ 0] = +M[ 0];
    T[ 1] = +M[ 4];
    T[ 2] = +M[ 8];
    T[ 3] = 0.0f;
    T[ 4] = +M[ 1];
    T[ 5] = +M[ 5];
    T[ 6] = +M[ 9];
    T[ 7] = 0.0f;
    T[ 8] = +M[ 2];
    T[ 9] = +M[ 6];
    T[10] = +M[10];
    T[11] = 0.0f;
    T[12] = 0.0f;
    T[13] = 0.0f;
    T[14] = 0.0f;
    T[15] = 1.0f;

    glMultMatrixf(T);
}

void app::view::mult_T() const
{
    // Apply the current view translation transform.

    glTranslatef(-M[12], -M[13], -M[14]);
}

void app::view::mult_V() const
{
    mult_R();
    mult_T();
}

//-----------------------------------------------------------------------------

void app::view::apply() const
{
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        mult_P();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();
        mult_V();
    }
}

//-----------------------------------------------------------------------------

void app::view::turn(float rx, float ry, float rz)
{
    const float x = M[12];
    const float y = M[13];
    const float z = M[14];

    Lmul_xlt_inv(M, x, y, z);
    Lmul_rot_mat(M, 0, 1, 0, ry);
    Lmul_xlt_mat(M, x, y, z);
    Rmul_rot_mat(M, 1, 0, 0, rx);
}

void app::view::move(float dx, float dy, float dz)
{
    Rmul_xlt_mat(M, dx, dy, dz);
}

//-----------------------------------------------------------------------------

void app::view::pick(float p[3], float v[3], int x, int y) const
{
    // Compute the screen-space pointer vector.

    float r = +(2.0f * x / w - 1.0f) * z * w / h;
    float u = -(2.0f * y / h - 1.0f) * z;

    // Transform the pointer to camera space and normalize.

    p[0] = M[12];
    p[1] = M[13];
    p[2] = M[14];

    v[0] = M[0] * r + M[4] * u - M[ 8] * n;
    v[1] = M[1] * r + M[5] * u - M[ 9] * n;
    v[2] = M[2] * r + M[6] * u - M[10] * n;

    float k = 1.0f / float(sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

//-----------------------------------------------------------------------------
