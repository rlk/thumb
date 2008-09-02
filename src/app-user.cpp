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

#include <sstream>
#include <iomanip>
#include <iostream>
#include <ode/ode.h>

#include "default.hpp"
#include "ogl-opengl.hpp"
#include "matrix.hpp"
#include "app-user.hpp"
#include "app-data.hpp"
#include "app-glob.hpp"
#include "app-prog.hpp"

//-----------------------------------------------------------------------------

app::user::user() :
    file("demo.xml"),
    root(0),
    prev(0),
    curr(0),
    next(0),
    pred(0),
    tt(0)
{
    const double S[16] = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0,
    };

    load_idt(current_M);
    load_idt(current_I);
    load_mat(current_S, S);

    // Initialize the demo input.

    root = find(file.get_head(), "demo");
    curr = find(root, "key");

    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);

    // Initialize the transformation using the initial state.

    double time;

    dostep(0.0, 0, time);
}

//-----------------------------------------------------------------------------

void app::user::get_point(double *P, const double *p,
                          double *V, const double *q) const
{
    double M[16];

    // Determine the point direction of the given quaternion.

    set_quaternion(M, q);

    // Transform the point position and direction to world space.

    mult_mat_vec3(P, current_M, p);
    mult_mat_vec3(V, current_M, M + 8);
}

//-----------------------------------------------------------------------------

app::node app::user::cycle_next(app::node n)
{
    // Return the next key, or the first key if there is no next.  O(1).

    app::node  node = app::next(root, n, "key");
    if (!node) node = app::find(root,    "key");

    return node;
}

app::node app::user::cycle_prev(app::node n)
{
    // Return the previous key, or the last key if there is no previous.  O(n).

    app::node last = 0;
    app::node node = 0;

    for (node = app::find(root,       "key"); node;
         node = app::next(root, node, "key"))
        if (cycle_next(node) == n)
            return node;
        else
            last = node;

    return last;
}

//-----------------------------------------------------------------------------

void app::user::turn(double rx, double ry, double rz, const double *R)
{
    double T[16];

    load_xps(T, R);

    mult_mat_mat(current_M, current_M, R);
    turn(rx, ry, rz);
    mult_mat_mat(current_M, current_M, T);

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

void app::user::home()
{
    load_idt(current_M);
    load_idt(current_I);
}

void app::user::tumble(const double *A,
                       const double *B)
{
    double T[16];

    load_xps(T, B);

    mult_mat_mat(current_M, current_M, A);
    mult_mat_mat(current_M, current_M, T);

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}

//-----------------------------------------------------------------------------

double app::user::interpolate(const char *name, double t)
{
    const double y0 = get_attr_f(prev, name, 0);
    const double y1 = get_attr_f(curr, name, 0);
    const double y2 = get_attr_f(next, name, 0);
    const double y3 = get_attr_f(pred, name, 0);

    return 0.5 * ((-1 * y0 +3 * y1 -3 * y2 + y3) * t * t * t +
                  ( 2 * y0 -5 * y1 +4 * y2 - y3) * t * t +
                  (-1 * y0             +y2     ) * t +
                  (         2 * y1             ));
}

/*
double app::user::interpolate(const char *name, double t)
{
    // Cubic Hermite keyframe interpolator.

    const double y0 = get_attr_f(prev, name, 0);
    const double y1 = get_attr_f(curr, name, 0);
    const double y2 = get_attr_f(next, name, 0);
    const double y3 = get_attr_f(pred, name, 0);

    double tension = 0.0;
    double bias    = 0.0;

    double t2 = t * t;
    double t3 = t * t2;

    double m0  = ((y1 - y0) * (1 + bias) * (1 - tension) / 2 +
                  (y2 - y1) * (1 - bias) * (1 - tension) / 2);
    double m1  = ((y2 - y1) * (1 + bias) * (1 - tension) / 2 +
                  (y3 - y2) * (1 - bias) * (1 - tension) / 2);

    double a0 =  2 * t3 - 3 * t2 + 1;
    double a1 =      t3 - 2 * t2 + t;
    double a2 =      t3 -     t2;
    double a3 = -2 * t3 + 3 * t2;

    return(a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2);
}
*/
/*
double app::user::interpolate(const char *name, double t)
{
    // Cubic Bezier keyframe interpolator.

    const double y0 = get_attr_f(prev, name, 0);
    const double y1 = get_attr_f(curr, name, 0);
    const double y2 = get_attr_f(next, name, 0);
    const double y3 = get_attr_f(pred, name, 0);

    const double a0 = y3 - y2 - y0 + y1;
    const double a1 = y0 - y1 - a0;
    const double a2 = y2 - y0;
    const double a3 = y1;

    const double t2 = t * t;

    return a0 * t * t2 + a1 * t2 + a2 * t + a3;
}
*/
/*
double app::user::interpolate(const char *name, double t)
{
    // Linear interpolator.

//  const double y0 = get_attr_f(prev, name, 0);
    const double y1 = get_attr_f(curr, name, 0);
    const double y2 = get_attr_f(next, name, 0);
//  const double y3 = get_attr_f(pred, name, 0);

    return y1 * (1.0 - t) + y2 * t;
}
*/

bool app::user::dostep(double dt, double ss, double &time)
{
    double p[3];
    double v[3];
    double q[4];

    // Compute the velocity over time dt.
/*
    v[0] = interpolate("x", tt + dt) - interpolate("x", tt);
    v[1] = interpolate("y", tt + dt) - interpolate("y", tt);
    v[2] = interpolate("z", tt + dt) - interpolate("z", tt);

    double s = sqrt(DOT3(v, v)) / dt;

    // Advance the bezier interpolator, scaled to match desired speed.

    if (dt > 0 && s > 0) tt += dt * ss / s;
*/

    tt += dt;

    // Advance the keys if necessary.

    while (tt > 1.0)
    {
        tt -= 1.0;

        curr = cycle_next(curr);
        prev = cycle_prev(curr);
        next = cycle_next(curr);
        pred = cycle_next(next);
    }

    // Interpolate the current demo state.  This could use some caching.

    p[0] = interpolate("x", tt);
    p[1] = interpolate("y", tt);
    p[2] = interpolate("z", tt);

    q[0] = interpolate("t", tt);
    q[1] = interpolate("u", tt);
    q[2] = interpolate("v", tt);
    q[3] = interpolate("w", tt);

    time = interpolate("time", tt);

    // Compute current transform from the interpolated values.

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    orthonormalize(current_M);

    load_inv(current_I, current_M);

    return true;
}

void app::user::gohalf()
{
    // Teleport half way to the next key.

    tt += (1.0 - tt) / 2.0;
}

void app::user::gonext()
{
    // Teleport to the next key.

    tt = 0.0;

    curr = cycle_next(curr);
    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);
}

void app::user::goprev()
{
    // Teleport to the previous key.

    tt = 0.0;

    curr = cycle_prev(curr);
    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);
}

void app::user::insert(double time)
{
    double q[4];

    get_quaternion(q, current_M);

    // Insert a new key after the current key.

    app::node node = create("key");

    set_attr_f(node, "x", current_M[12]);
    set_attr_f(node, "y", current_M[13]);
    set_attr_f(node, "z", current_M[14]);

    set_attr_f(node, "t", q[0]);
    set_attr_f(node, "u", q[1]);
    set_attr_f(node, "v", q[2]);
    set_attr_f(node, "w", q[3]);

    set_attr_f(node, "time", time);

    app::insert(root, curr, node);

    curr = node;
    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);

    tt = 0.0;
}

void app::user::remove()
{
    app::node node = cycle_next(curr);

    // Remove the current key (if it's not the only one left).

    if (node != curr)
    {
        app::remove(curr);

        curr = node;
        prev = cycle_prev(curr);
        next = cycle_next(curr);
        pred = cycle_next(next);

        tt = 0.0;
    }
}

//-----------------------------------------------------------------------------
