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

#include <sys/time.h>

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
#include "app-conf.hpp"
#include "app-prog.hpp"

// TODO: The use of set() is haphazzard.  current_M/I are accessed directly.
// Clarify this interface.

//-----------------------------------------------------------------------------

app::user::user() :
    move_rate(1.0),
    turn_rate(1.0),
    move_rate_k(1.0),
    turn_rate_k(1.0),
    file(DEFAULT_DEMO_FILE),
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
    load_mat(current_S, S);

    home();

    move_rate_k = ::conf->get_f("view_move_rate");
    turn_rate_k = ::conf->get_f("view_turn_rate");

    fly_r_min = ::conf->get_f("fly_r_min", 6372797.0);
    fly_r_max = ::conf->get_f("fly_r_max", 6381641.0);

    // Initialize the demo input.

    root = file.get_root().find("demo");
    curr = root.find("key");

    prev = cycle_prev(curr);
    next = cycle_next(curr);
    pred = cycle_next(next);

    // Initialize the transformation using the initial state.

    int opts;

    set_state(curr, opts);

    home();
}

void app::user::set(const double *p, const double *q, double t)
{
    // Compute the current transform and inverse from the given values.

    if (q)
        set_quaternion(current_M, q);

    if (p)
    {
        current_M[12] = p[0];
        current_M[13] = p[1];
        current_M[14] = p[2];
    }

    if (p || q)
    {
        orthonormalize(current_M);
        load_inv(current_I, current_M);
    }

    // Compute the current lighting vector from the given time.
    // TODO: compute this from lat/lon/date/time.

    if (t)
    {
        current_t = t;

        double M[16], L[3] = { 0.0, 0.0, 8192.0 }; // HACK

        load_rot_mat(M, 1.0, 0.0, 0.0, 120.0);
        Rmul_rot_mat(M, 0.0, 1.0, 0.0, 360.0 * t / (24.0 * 60.0 * 60.0));

        mult_mat_vec3(current_L, M, L);
/*
        double phi   = RAD(30.0);
        double theta = RAD(60.0);

        current_L[0] = (sin(phi) * sin(theta));
        current_L[1] = (           cos(theta));
        current_L[2] = (cos(phi) * sin(theta));
*/
    }
}

//-----------------------------------------------------------------------------

void app::user::get_point(double *P, const double *p,
                          double *V, const double *q) const
{
    double M[16], v[3];

    // Determine the point direction of the given quaternion.

    set_quaternion(M, q);

    v[0] = -M[ 8];
    v[1] = -M[ 9];
    v[2] = -M[10];

    // Transform the point position and direction to world space.

    mult_mat_vec3(P, current_M, p);
    mult_xps_vec3(V, current_I, v);
}

//-----------------------------------------------------------------------------

app::node app::user::cycle_next(app::node n)
{
    // Return the next key, or the first key if there is no next.  O(1).

    app::node c = root.next(n, "key");
    if (!c)   c = root.find(   "key");

    return c;
}

app::node app::user::cycle_prev(app::node n)
{
    // Return the previous key, or the last key if there is no previous.  O(n).

    app::node l(0);
    app::node c(0);

    for (c = root.find("key"); c; c = root.next(c, "key"))

        if (cycle_next(c) == n)
            return c;
        else
            l = c;

    return l;
}

//-----------------------------------------------------------------------------

void app::user::turn(double rx, double ry, double rz, const double *R)
{
    // Turn in the given coordinate system.

    double T[16];

    load_xps(T, R);

    if (::prog->get_option(6))
    {
        mult_mat_mat(current_M, current_M, R);
        turn(rx, ry, rz);
        mult_mat_mat(current_M, current_M, T);
    }
    else
        turn(0, ry, 0);

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

void app::user::pass(double dt)
{
    current_t += dt;

    set(0, 0, current_t);
}

void app::user::home()
{
    load_idt(current_M);
    load_idt(current_I);

    // HACK

    double M[16];

    M[0] =  1; M[4] =  0; M[ 8] =  0;  M[12] =  0;
    M[1] =  0; M[5] =  0; M[ 9] = -1;  M[13] =  0;
    M[2] =  0; M[6] = -1; M[10] =  0;  M[14] =  2150000;
    M[3] =  0; M[7] =  0; M[11] =  0;  M[15] =  1;

    load_mat(current_M, M);
    load_inv(current_I, M);

//  current_M[13] =  1.759;
//  current_I[13] = -1.759;

//  struct timeval tv;

//  gettimeofday(&tv, 0);

//  set(0, 0, tv.tv_sec + tv.tv_usec * 0.000001);
    set(0, 0, 1);
}

/*
void app::user::orbit(double a, const double *p)
{
    const double rx = current_M[0];
    const double ry = current_M[1];
    const double rz = current_M[2];

    Lmul_xlt_inv(current_M, p[0], p[1], p[2]);
    Lmul_rot_mat(current_M, rx, ry, rz, a);
    Lmul_xlt_mat(current_M, p[0], p[1], p[2]);

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}
*/

void app::user::fly(double dp, double dy, double dz)
{
    // TODO: flyto here?

    double X[3];
    double Z[3];
    double P[3];
    double N[3];

    X[0] = current_M[ 0];
    X[1] = current_M[ 1];
    X[2] = current_M[ 2];

    Z[0] = current_M[ 8];
    Z[1] = current_M[ 9];
    Z[2] = current_M[10];

    P[0] = current_M[12];
    P[1] = current_M[13];
    P[2] = current_M[14];

    N[0] = current_M[12];
    N[1] = current_M[13];
    N[2] = current_M[14];

    normalize(N);

    // Yaw about the "out" vector.

    if (fabs(dy) > 0.25)
        Lmul_rot_mat(current_M, N[0], N[1], N[2], dy * get_turn_rate());

    // Pitch about the "right" vector.

    if (fabs(dp) > 0.25)
    {
        Lmul_xlt_inv(current_M, P[0], P[1], P[2]);
        Lmul_rot_mat(current_M, X[0], X[1], X[2], dp * get_turn_rate());
        Lmul_xlt_mat(current_M, P[0], P[1], P[2]);
    }

    // Move about the "forward" vector.

    Lmul_xlt_mat(current_M,
                 Z[0] * dz * get_move_rate(),
                 Z[1] * dz * get_move_rate(),
                 Z[2] * dz * get_move_rate());

    // Determine the current pitch and altitude.

    X[0] = current_M[ 0];
    X[1] = current_M[ 1];
    X[2] = current_M[ 2];

    P[0] = current_M[12];
    P[1] = current_M[13];
    P[2] = current_M[14];

    N[0] = current_M[12];
    N[1] = current_M[13];
    N[2] = current_M[14];

    normalize(N);

    double p = -DEG(asin(DOT3(N, current_M + 8)));
    double r = sqrt(DOT3(P, P));

    // Clamp the pitch and altitude.

    const double rr0 = (fly_r_max - fly_r_min) / 100.0;
    const double rr1 = (fly_r_max - fly_r_min) /  10.0;

    double p0 = -30.0 * (1.0 - 1.0 / (1.0 + (+r - fly_r_min) / rr0));
    double p1 =  30.0 * (1.0 - 1.0 / (1.0 + (-r + fly_r_max) / rr1));

    r = std::max(r, fly_r_min);
    r = std::min(r, fly_r_max);
    p = std::max(p, p0);
    p = std::min(p, p1);

    // Compute the transform with the clamped pitch and altitude.

    current_M[4] = N[0];
    current_M[5] = N[1];
    current_M[6] = N[2];
    crossprod(current_M + 8, current_M + 0, current_M + 4);

    Lmul_xlt_inv(current_M, P[0], P[1], P[2]);
    Lmul_rot_mat(current_M, X[0], X[1], X[2], p);
    Lmul_xlt_mat(current_M, P[0], P[1], P[2]);

    current_M[12] = N[0] * r;
    current_M[13] = N[1] * r;
    current_M[14] = N[2] * r;

    // 

    orthonormalize(current_M);

    load_inv(current_I, current_M);
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

    const double y1 = A.get_f(name, 0);
    const double y2 = B.get_f(name, 0);

    double k = cubic(cubic(t));

    return y1 * (1.0 - k) + y2 * k;
}

void app::user::erp_state(app::node A,
                          app::node B, double tt, int &opts)
{
    // Apply the interpolation of the given state nodes.

    double p[3];
    double q[4];
    double time;

    p[0] = interpolate(A, B, "x", tt);
    p[1] = interpolate(A, B, "y", tt);
    p[2] = interpolate(A, B, "z", tt);

    q[0] = interpolate(A, B, "t", tt);
    q[1] = interpolate(A, B, "u", tt);
    q[2] = interpolate(A, B, "v", tt);
    q[3] = interpolate(A, B, "w", tt);

    time = interpolate(A, B, "time", tt);

    if (tt < 0.5)
        opts = A.get_i("opts", 0);
    else
        opts = B.get_i("opts", 0);

    set(p, q, time);
}

void app::user::set_state(app::node A, int &opts)
{
    // Apply the given state node.

    double p[3];
    double q[4];
    double time;

    p[0] = A.get_f("x", 0);
    p[1] = A.get_f("y", 0);
    p[2] = A.get_f("z", 0);

    q[0] = A.get_f("t", 0);
    q[1] = A.get_f("u", 0);
    q[2] = A.get_f("v", 0);
    q[3] = A.get_f("w", 0);

    time = A.get_i("time", 0);
    opts = A.get_i("opts", 0);

    set(p, q, time);
}

bool app::user::dostep(double dt, int &opts)
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
        set_state(curr, opts);
        tt = 0.0;
        return true;
    }

    // If we're going forward, stop at the next state.

    if (dt > 0.0 && tt > 1.0)
    {
        stopped = true;
        set_state(next, opts);
        tt = 1.0;
        return true;
    }

    // Otherwise just interpolate the two.
    
    erp_state(curr, next, tt, opts);

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

void app::user::insert(int opts)
{
    if (stopped)
    {
        // If we're waiting at the end of a step, advance.

        if (tt == 1.0)
            gonext();

        double q[4];

        get_quaternion(q, current_M);

        // Insert a new key after the current key.

        app::node c("key");

        c.set_f("x", current_M[12]);
        c.set_f("y", current_M[13]);
        c.set_f("z", current_M[14]);

        c.set_f("t", q[0]);
        c.set_f("u", q[1]);
        c.set_f("v", q[2]);
        c.set_f("w", q[3]);

        c.set_f("time", current_t);
        c.set_i("opts", opts);

        c.insert(root, curr);

        curr = c;
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
            curr.remove();

            curr = node;
            prev = cycle_prev(curr);
            next = cycle_next(curr);
            pred = cycle_next(next);

            tt = 0.0;
        }
    }
}

//-----------------------------------------------------------------------------

void app::user::auto_step(double dt)
{
    if (auto_b)
    {
        double n[3];
        double v[3];
        double u[3];

        // Find the unit vector pointing out of the planet.

        n[0] = current_M[12];
        n[1] = current_M[13];
        n[2] = current_M[14];

        normalize(n);

        // Find the vector to the destination.

        v[0] = auto_n1[0] * fly_r_min - current_M[12];
        v[1] = auto_n1[1] * fly_r_min - current_M[13];
        v[2] = auto_n1[2] * fly_r_min - current_M[14];

        // Find the yaw angle to the destination vector.

        double k = DOT3(current_M + 4, v);

        u[0] = v[0] - current_M[4] * k;
        u[1] = v[1] - current_M[5] * k;
        u[2] = v[2] - current_M[6] * k;

        normalize(u);

        double dy, y = -DEG(asin(DOT3(u, current_M)));

        dy = y;
        dy = std::min(dy,  45.0 * dt);
        dy = std::max(dy, -45.0 * dt);

        // Find the starting and current angles.
/*
        double a  = DEG(acos(DOT3(auto_n1, n)));
        double a0 = DEG(acos(DOT3(auto_n1, auto_n0)));
*/
        // Find the pitch angle.
/*
        double t = 1.0 - pow(2.0 * a / a0 - 1.0, 2.0);
        double r = fly_r_min + t * (fly_r_max - fly_r_min);

        double dr = r - sqrt(DOT3(current_M + 12, current_M + 12));

        double dp, p = dr;

        dp = p;
        dp = std::min(dp,  15.0 * dt);
        dp = std::max(dp, -15.0 * dt);
*/
        fly(0, dy, -2.0 * dt);
    }
}

void app::user::auto_init(const double *n)
{
    auto_n0[0] = current_M[12];
    auto_n0[0] = current_M[13];
    auto_n0[0] = current_M[14];

    auto_n1[0] = n[0];
    auto_n1[1] = n[1];
    auto_n1[2] = n[2];

    normalize(auto_n0);
    normalize(auto_n1);

    auto_b = true;
}

void app::user::auto_stop()
{
    auto_b = false;
}

//-----------------------------------------------------------------------------

void app::user::draw() const
{
    // This is a view matrix rather than a model matrix.  It must be inverse.

    glLoadMatrixd(current_I);
}

//-----------------------------------------------------------------------------
