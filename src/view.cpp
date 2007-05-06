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
    load_idt(default_M);
    load_idt(current_M);

    move(0.0f, 3.0f, 5.0f);
}

//-----------------------------------------------------------------------------

void app::view::set(const float M[16])
{
    memcpy(current_M, M, 16 * sizeof (float));
}

void app::view::clr()
{
    memcpy(current_M, default_M, 16 * sizeof (float));
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
    glMultMatrixf(current_M);
}

void app::view::mult_R() const
{
    float T[16];

    // Apply the current view rotation transform.

    T[ 0] = +current_M[ 0];
    T[ 1] = +current_M[ 4];
    T[ 2] = +current_M[ 8];
    T[ 3] = 0.0f;
    T[ 4] = +current_M[ 1];
    T[ 5] = +current_M[ 5];
    T[ 6] = +current_M[ 9];
    T[ 7] = 0.0f;
    T[ 8] = +current_M[ 2];
    T[ 9] = +current_M[ 6];
    T[10] = +current_M[10];
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

    glTranslatef(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
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

void app::view::push() const
{
    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPushMatrix();
    }
}

void app::view::pop() const
{
    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------

void app::view::turn(float rx, float ry, float rz)
{
    const float x = default_M[12];
    const float y = default_M[13];
    const float z = default_M[14];

    Lmul_xlt_inv(default_M, x, y, z);
    Lmul_rot_mat(default_M, 0, 1, 0, ry);
    Lmul_xlt_mat(default_M, x, y, z);
    Rmul_rot_mat(default_M, 1, 0, 0, rx);

    clr();
}

void app::view::move(float dx, float dy, float dz)
{
    Rmul_xlt_mat(default_M, dx, dy, dz);

    clr();
}

//-----------------------------------------------------------------------------

void app::view::pick(float p[3], float v[3], int x, int y) const
{
    // Compute the screen-space pointer vector.

    float r = +(2.0f * x / w - 1.0f) * z * w / h;
    float u = -(2.0f * y / h - 1.0f) * z;

    // Transform the pointer to camera space and normalize.

    p[0] = current_M[12];
    p[1] = current_M[13];
    p[2] = current_M[14];

    v[0] = current_M[0] * r + current_M[4] * u - current_M[ 8] * n;
    v[1] = current_M[1] * r + current_M[5] * u - current_M[ 9] * n;
    v[2] = current_M[2] * r + current_M[6] * u - current_M[10] * n;

    float k = 1.0f / float(sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

//-----------------------------------------------------------------------------
