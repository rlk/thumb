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

#include <iostream>
#include <ode/ode.h>

#include "opengl.hpp"
#include "matrix.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

app::view::view(int w, int h, double n, double f, double z) :
    w(w), h(h), n(n), f(f), z(z)
{
    load_idt(default_M);
    load_idt(current_M);
}

//-----------------------------------------------------------------------------

void app::view::set(const double *M)
{
    load_mat(current_M, M);
}

void app::view::get(double *M)
{
    load_mat(M, current_M);
}

void app::view::clr()
{
    load_mat(current_M, default_M);
}

//-----------------------------------------------------------------------------

void app::view::mult_S() const
{
    double T[16] = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0,
    };

    glMultMatrixd(T);
}

void app::view::mult_P() const
{
    double A = double(w) / double(h);
    double N = double(n) / 2;

    // Apply the perspective projection.

    glFrustum(-A * z * N, +A * z * N, -z * N, +z * N, N, f);
}

void app::view::mult_O() const
{
    // Apply the orthogonal projection.

    glOrtho(0, w, h, 0, 0, 1);
}

void app::view::mult_M() const
{
    glMultMatrixd(current_M);
}

void app::view::mult_R() const
{
    double T[16];

    // Apply the current view rotation transform.

    T[ 0] = +current_M[ 0];
    T[ 1] = +current_M[ 4];
    T[ 2] = +current_M[ 8];
    T[ 3] = 0;
    T[ 4] = +current_M[ 1];
    T[ 5] = +current_M[ 5];
    T[ 6] = +current_M[ 9];
    T[ 7] = 0;
    T[ 8] = +current_M[ 2];
    T[ 9] = +current_M[ 6];
    T[10] = +current_M[10];
    T[11] = 0;
    T[12] = 0;
    T[13] = 0;
    T[14] = 0;
    T[15] = 1;

    glMultMatrixd(T);
}

void app::view::mult_T() const
{
    // Apply the current view translation transform.

    glTranslated(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
}

void app::view::mult_V() const
{
    mult_R();
    mult_T();
}

//-----------------------------------------------------------------------------

void app::view::range(double N, double F)
{
    std::cout << N << " " << F << std::endl;

    if (N < F)
    {
        n = N;
        f = F;
    }
    else
    {
        n =  1.0;
        f = 10.0;
    }
}

void app::view::draw() const
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

void app::view::turn(double rx, double ry, double rz)
{
    // Grab basis vectors (which change during transform).

    const double xx = default_M[ 0];
    const double xy = default_M[ 1];
    const double xz = default_M[ 2];

    const double yx = default_M[ 4];
    const double yy = default_M[ 5];
    const double yz = default_M[ 6];

    const double zx = default_M[ 8];
    const double zy = default_M[ 9];
    const double zz = default_M[10];

    const double px = default_M[12];
    const double py = default_M[13];
    const double pz = default_M[14];

    // Apply a local rotation transform.

    Lmul_xlt_inv(default_M, px, py, pz);
    Lmul_rot_mat(default_M, xx, xy, xz, rx);
    Lmul_rot_mat(default_M, yx, yy, yz, ry);
    Lmul_rot_mat(default_M, zx, zy, zz, rz);
    Lmul_xlt_mat(default_M, px, py, pz);

    clr();
}

void app::view::move(double dx, double dy, double dz)
{
    Rmul_xlt_mat(default_M, dx, dy, dz);

    clr();
}

void app::view::zoom(double dz)
{
    if (z *  dz > 0.01)
        z *= dz;
}

//-----------------------------------------------------------------------------

void app::view::world_frustum(double *V) const
{
    const double A = double(w) / double(h);
    const double Z = double(z);

    // View plane.

    V[ 0] =  0;
    V[ 1] =  0;
    V[ 2] = -1;
    V[ 3] =  0;

    // Left plane.

    V[ 4] =  1;
    V[ 5] =  0;
    V[ 6] = -Z * A;
    V[ 7] =  0;

    // Right plane.

    V[ 8] = -1;
    V[ 9] =  0;
    V[10] = -Z * A;
    V[11] =  0;

    // Bottom plane.

    V[12] =  0;
    V[13] =  1;
    V[14] = -Z;
    V[15] =  0;

    // Top plane.

    V[16] =  0;
    V[17] = -1;
    V[18] = -Z;
    V[19] =  0;

    // Normalize all plane vectors.

    normalize(V +  0);
    normalize(V +  4);
    normalize(V +  8);
    normalize(V + 12);
}

void app::view::plane_frustum(double *V) const
{
    const double A = double(w) / double(h);
    const double Z = double(z);

    // View plane.

    V[ 0] = -current_M[ 8] * Z;
    V[ 1] = -current_M[ 9] * Z;
    V[ 2] = -current_M[10] * Z;

    // Left plane.

    V[ 4] =  current_M[ 0] + V[0] * A;
    V[ 5] =  current_M[ 1] + V[1] * A;
    V[ 6] =  current_M[ 2] + V[2] * A;

    // Right plane.

    V[ 8] = -current_M[ 0] + V[0] * A;
    V[ 9] = -current_M[ 1] + V[1] * A;
    V[10] = -current_M[ 2] + V[2] * A;

    // Bottom plane.

    V[12] =  current_M[ 4] + V[0];
    V[13] =  current_M[ 5] + V[1];
    V[14] =  current_M[ 6] + V[2];

    // Top plane.

    V[16] = -current_M[ 4] + V[0];
    V[17] = -current_M[ 5] + V[1];
    V[18] = -current_M[ 6] + V[2];

    // Normalize all plane vectors.

    normalize(V +  0);
    normalize(V +  4);
    normalize(V +  8);
    normalize(V + 12);
    normalize(V + 16);

    // Compute all plane offsets.

    V[ 3] = -DOT3(V +  0, current_M + 12);
    V[ 7] = -DOT3(V +  4, current_M + 12);
    V[11] = -DOT3(V +  8, current_M + 12);
    V[15] = -DOT3(V + 12, current_M + 12);
    V[19] = -DOT3(V + 16, current_M + 12);
}

void app::view::point_frustum(double *V) const
{
    const double a = double(w) / double(h);

    const double *P = current_M + 12;
    const double *B = current_M +  8;

    double R[3];
    double U[3];

    R[0] = current_M[0] * a * z;
    R[1] = current_M[1] * a * z;
    R[2] = current_M[2] * a * z;
    
    U[0] = current_M[4]     * z;
    U[1] = current_M[5]     * z;
    U[2] = current_M[6]     * z;
    
    // The points of the near plane.

    V[ 0] = P[0] + n * (-R[0] - U[0] - B[0]);
    V[ 1] = P[1] + n * (-R[1] - U[1] - B[1]);
    V[ 2] = P[2] + n * (-R[2] - U[2] - B[2]);

    V[ 3] = P[0] + n * (+R[0] - U[0] - B[0]);
    V[ 4] = P[1] + n * (+R[1] - U[1] - B[1]);
    V[ 5] = P[2] + n * (+R[2] - U[2] - B[2]);

    V[ 6] = P[0] + n * (+R[0] + U[0] - B[0]);
    V[ 7] = P[1] + n * (+R[1] + U[1] - B[1]);
    V[ 8] = P[2] + n * (+R[2] + U[2] - B[2]);

    V[ 9] = P[0] + n * (-R[0] + U[0] - B[0]);
    V[10] = P[1] + n * (-R[1] + U[1] - B[1]);
    V[11] = P[2] + n * (-R[2] + U[2] - B[2]);

    // The points of the far plane.

    V[12] = P[0] + f * (-R[0] - U[0] - B[0]);
    V[13] = P[1] + f * (-R[1] - U[1] - B[1]);
    V[14] = P[2] + f * (-R[2] - U[2] - B[2]);

    V[15] = P[0] + f * (+R[0] - U[0] - B[0]);
    V[16] = P[1] + f * (+R[1] - U[1] - B[1]);
    V[17] = P[2] + f * (+R[2] - U[2] - B[2]);

    V[18] = P[0] + f * (+R[0] + U[0] - B[0]);
    V[19] = P[1] + f * (+R[1] + U[1] - B[1]);
    V[20] = P[2] + f * (+R[2] + U[2] - B[2]);

    V[21] = P[0] + f * (-R[0] + U[0] - B[0]);
    V[22] = P[1] + f * (-R[1] + U[1] - B[1]);
    V[23] = P[2] + f * (-R[2] + U[2] - B[2]);
}

//-----------------------------------------------------------------------------

void app::view::pick(double *p, double *v, int x, int y) const
{
    // Compute the screen-space pointer vector.

    double r = +(2.0 * x / w - 1.0) * z * w / h;
    double u = -(2.0 * y / h - 1.0) * z;

    // Transform the pointer to camera space and normalize.

    p[0] = current_M[12];
    p[1] = current_M[13];
    p[2] = current_M[14];

    v[0] = current_M[0] * r + current_M[4] * u - current_M[ 8] * n;
    v[1] = current_M[1] * r + current_M[5] * u - current_M[ 9] * n;
    v[2] = current_M[2] * r + current_M[6] * u - current_M[10] * n;

    double k = 1.0 / sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);

    v[0] *= k;
    v[1] *= k;
    v[2] *= k;
}

double app::view::dist(double *p) const
{
    return distance(p, current_M + 12);
}

//-----------------------------------------------------------------------------
