//  Copyright (C) 2005-2011 Robert Kooima
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

#include <app-default.hpp>
#include <ogl-opengl.hpp>
#include <etc-math.hpp>
#include <app-user.hpp>
#include <app-data.hpp>
#include <app-conf.hpp>

//-----------------------------------------------------------------------------

app::user::user()
{
    load_idt(current_M);
    load_idt(current_I);
}

//-----------------------------------------------------------------------------

void app::user::get_point(double *P, const double *p,
                          double *V, const double *q) const
{
    double M[16], v[3];

    // Determine the point direction of the given quaternion.

    quat_to_mat(M, q);

    v[0] = -M[ 8];
    v[1] = -M[ 9];
    v[2] = -M[10];

    // Transform the point position and direction to world space.

    mult_mat_vec3(P, current_M, p);
    mult_xps_vec3(V, current_I, v);
}

//-----------------------------------------------------------------------------

void app::user::turn(double rx, double ry, double rz, const double *R)
{
    // Turn in the given coordinate system.

    double T[16];

    load_xps(T, R);

#if CONFIG_EASY_FLIGHT
    turn(0, ry, 0);
#else
    mult_mat_mat(current_M, current_M, R);
    turn(rx, ry, rz);
    mult_mat_mat(current_M, current_M, T);
#endif

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}

void app::user::turn(double rx, double ry, double rz)
{
    // Grab basis vectors (which change during transform).

    const double xx = current_M[ 0];
    const double xy = current_M[ 1];
    const double xz = current_M[ 2];

    const double yx = current_M[ 4];
    const double yy = current_M[ 5];
    const double yz = current_M[ 6];

    const double zx = current_M[ 8];
    const double zy = current_M[ 9];
    const double zz = current_M[10];

    const double px = current_M[12];
    const double py = current_M[13];
    const double pz = current_M[14];

    // Apply a local rotation transform.

    Lmul_xlt_inv(current_M, px, py, pz);
    Lmul_rot_mat(current_M, xx, xy, xz, rx);
    Lmul_rot_mat(current_M, yx, yy, yz, ry);
    Lmul_rot_mat(current_M, zx, zy, zz, rz);
    Lmul_xlt_mat(current_M, px, py, pz);

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}

void app::user::move(double dx, double dy, double dz)
{
    Rmul_xlt_mat(current_M, dx, dy, dz);

    load_inv(current_I, current_M);
}

void app::user::look(double dt, double dp)
{
    const double x = current_M[12];
    const double y = current_M[13];
    const double z = current_M[14];

    // Apply a mouselook-style local rotation transform.

    Lmul_xlt_inv(current_M, x, y, z);
    Lmul_rot_mat(current_M, 0, 1, 0, dt);
    Lmul_xlt_mat(current_M, x, y, z);
    Rmul_rot_mat(current_M, 1, 0, 0, dp);

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}

void app::user::home()
{
    load_idt(current_M);
    load_idt(current_I);
}

void app::user::set_M(const double *M)
{
    load_mat(current_M, M);
    load_inv(current_I, M);
}

//-----------------------------------------------------------------------------

void app::user::draw() const
{
    // This is a view matrix rather than a model matrix. It must be inverse.

    glLoadMatrixd(current_I);
}

//-----------------------------------------------------------------------------
