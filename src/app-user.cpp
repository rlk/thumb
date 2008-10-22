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
    tt(0),
    stopped(true)
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
    int    opts;

    set_state(curr, time, opts);
}

void app::user::set(const double p[3], const double q[4])
{
    // Compute the current transform and inverse from the given values.

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    orthonormalize(current_M);

    load_inv(current_I, current_M);
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

/*
const double *app::user::get_M()
{
    load_mat(cache_M, current_M);
    Lmul_rot_mat(cache_M, 1.0, 0.0, 0.0, -45.0);
    return cache_M;
}

const double *app::user::get_I()
{
    load_mat(cache_I, current_I);
    Rmul_rot_inv(cache_I, 1.0, 0.0, 0.0, -45.0);
    return cache_I;
}
*/
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

static double cubic(double t)
{
    return 3 * t * t - 2 * t * t * t;
}

double app::user::interpolate(app::node A,
                              app::node B, const char *name, double t)
{
    // Cubic interpolator.

    const double y1 = get_attr_f(A, name, 0);
    const double y2 = get_attr_f(B, name, 0);

    double k = cubic(cubic(t));

    return y1 * (1.0 - k) + y2 * k;
}

void app::user::erp_state(app::node A,
                          app::node B, double tt, double &time, int &opts)
{
    // Apply the interpolation of the given state nodes.

    double p[3];
    double q[4];

    p[0] = interpolate(A, B, "x", tt);
    p[1] = interpolate(A, B, "y", tt);
    p[2] = interpolate(A, B, "z", tt);

    q[0] = interpolate(A, B, "t", tt);
    q[1] = interpolate(A, B, "u", tt);
    q[2] = interpolate(A, B, "v", tt);
    q[3] = interpolate(A, B, "w", tt);

    time = interpolate(A, B, "time", tt);

    if (tt < 0.5)
        opts = get_attr_f(A, "opts", 0);
    else
        opts = get_attr_f(B, "opts", 0);

    set(p, q);
}

void app::user::set_state(app::node A, double &time, int &opts)
{
    // Apply the given state node.

    double p[3];
    double q[4];

    p[0] = get_attr_f(A, "x", 0);
    p[1] = get_attr_f(A, "y", 0);
    p[2] = get_attr_f(A, "z", 0);

    q[0] = get_attr_f(A, "t", 0);
    q[1] = get_attr_f(A, "u", 0);
    q[2] = get_attr_f(A, "v", 0);
    q[3] = get_attr_f(A, "w", 0);

    time = get_attr_f(A, "time", 0);
    opts = get_attr_f(A, "opts", 0);

    set(p, q);
}

bool app::user::dostep(double dt, double ss, double &time, int &opts)
{
    // If we're starting backward, advance to the previous state.

    if (stopped && dt < 0.0)
    {
        if (tt == 0.0)
        {
            stopped = false;
            goprev();
            tt = 1.0;
        }
    }

    // If we're starting foreward, advance to the next state.

    if (stopped && dt > 0.0)
    {
        if (tt == 1.0)
        {
            stopped = false;
            gonext();
            tt = 0.0;
        }
    }

    tt += dt;

    // If we're going backward, stop at the current state.

    if (dt < 0.0 && tt < 0.0)
    {
        stopped = true;
        set_state(curr, time, opts);
        tt = 0.0;
        return true;
    }

    // If we're going forward, stop at the next state.

    if (dt > 0.0 && tt > 1.0)
    {
        stopped = true;
        set_state(next, time, opts);
        tt = 1.0;
        return true;
    }

    // Otherwise just interpolate the two.
    
    erp_state(curr, next, tt, time, opts);

    return false;
}

void app::user::gohalf()
{
    // Teleport half way to the next key.

    tt += (1.0 - tt) / 2.0;
}

void app::user::gonext()
{
    // Teleport to the next key.

    curr = cycle_next(curr);
    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);
}

void app::user::goprev()
{
    // Teleport to the previous key.

    curr = cycle_prev(curr);
    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);
}

void app::user::insert(double time, int opts)
{
    if (stopped)
    {
        // If we're waiting at the end of a step, advance.

        if (tt == 1.0)
            gonext();

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
        set_attr_d(node, "opts", opts);

        app::insert(root, curr, node);

        curr = node;
        prev = cycle_prev(curr);
        next = cycle_next(curr);
        pred = cycle_next(next);

        tt = 0.0;
    }
}

void app::user::remove()
{
    if (stopped)
    {
        // If we're waiting at the end of a step, advance.

        if (tt == 1.0)
            gonext();

        // Remove the current key (if it's not the only one left).

        app::node node = cycle_next(curr);

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
}

//-----------------------------------------------------------------------------
