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
#include "opengl.hpp"
#include "matrix.hpp"
#include "user.hpp"
#include "data.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

static double get_real_attr(mxml_node_t *node, const char *name)
{
    if (const char *c = mxmlElementGetAttr(node, name))
        return atof(c);
    else
        return 0;
}

static void set_real_attr(mxml_node_t *node, const char *name, double k)
{
    std::ostringstream value;

    value << std::setprecision(3) << k;

    mxmlElementSetAttr(node, name, value.str().c_str());
}

//-----------------------------------------------------------------------------

app::user::user() :
    head(0),
    root(0),
    curr(0),
    t0(0),
    tt(0),
    t1(0),
    current_a0(0), current_a1(0), current_a(0),
    current_t0(0), current_t1(0), current_t(0)
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

    load_idt(current_M0);
    load_idt(current_M1);
 
    init();
    load();

    gonext(0);
}

app::user::~user()
{
    save();
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

void app::user::set_default()
{
    memcpy(default_M, current_M, 16 * sizeof (double));
    memcpy(default_I, current_I, 16 * sizeof (double));
}

void app::user::get_default()
{
    memcpy(current_M, default_M, 16 * sizeof (double));
    memcpy(current_I, default_I, 16 * sizeof (double));
}

//-----------------------------------------------------------------------------

void app::user::init()
{
    head = mxmlNewElement(NULL, "?xml");
    root = mxmlNewElement(head, "demo");

    mxmlElementSetAttr(head, "version",  "1.0");
    mxmlElementSetAttr(head, "?", NULL);

    dirty = false;
}

//-----------------------------------------------------------------------------

static mxml_type_t load_cb(mxml_node_t *node)
{
    std::string name(node->value.element.name);

    if (name == "key")
        return MXML_REAL;
    else
        return MXML_ELEMENT;
}

bool app::user::load()
{
    if (head) mxmlDelete(head);

    const char *buff;

    if ((buff = (const char *) ::data->load(DEFAULT_DEMO_FILE)))
    {
        head = mxmlLoadString(NULL, buff, load_cb);
        root = mxmlFindElement(head, head, "demo", 0, 0, MXML_DESCEND_FIRST);
        curr = 0;
    }

    ::data->free(DEFAULT_DEMO_FILE);

    dirty = false;

    return root ? true : false;
}

//-----------------------------------------------------------------------------

static const char *save_cb(mxml_node_t *node, int where)
{
    std::string name(node->value.element.name);

    switch (where)
    {
    case MXML_WS_AFTER_OPEN:  return "\n";
    case MXML_WS_AFTER_CLOSE: return "\n";

    case MXML_WS_BEFORE_OPEN:
        if (name == "key") return "  ";
        break;
    }

    return NULL;
}

void app::user::save()
{
    if (dirty)
    {
        char *buff;

        if ((buff = mxmlSaveAllocString(head, save_cb)))
        {
            ::data->save(DEFAULT_DEMO_FILE, buff);
            free(buff);
        }

        dirty = false;
    }
}

//-----------------------------------------------------------------------------

void app::user::turn(double rx, double ry, double rz, const double *R)
{
    double T[16];

    load_xps(T, R);

    mult_mat_mat(current_M, current_M, R);
    turn(rx, ry, rz);
    mult_mat_mat(current_M, current_M, T);

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

bool app::user::dostep(double dt, const double *p, double& a, double& t)
{
    tt += dt;

    // Interpolate between the current endpoint transforms.

    if (t0 <= tt && tt <= t1)
    {
        double k = (tt - t0) / (t1 - t0);
        double T = 3 * k * k - 2 * k * k * k;

        a = current_a = (1 - T) * current_a0 + T * current_a1;
        t = current_t = (1 - T) * current_t0 + T * current_t1;

        slerp(current_M0, current_M1, p, T);

        return false;
    }

    // Clamp to the beginning of the interpolation.

    else if (tt < t0)
    {
        load_mat(current_M, current_M0);
        load_inv(current_I, current_M0);
        a = current_a = current_a0;
        t = current_t = current_t0;

        return false;
    }

    // Clamp to the end of the interpolation.

    else if (tt > t1)
    {
        return true;
/*
        load_mat(current_M, current_M1);
        load_inv(current_I, current_M1);
        a = current_a = current_a1;
        t = current_t = current_t1;

        return true;
*/
    }

    return true;
}

void app::user::gocurr(double dt)
{
    t0 = tt;
    t1 = tt + dt;

    current_a0 = current_a;
    current_t0 = current_t;

    load_mat(current_M0, current_M);
    load_mat(current_M1, current_M);

    // Load the destination matrix from the XML element.

    if (curr)
    {
        current_M1[ 0] = get_real_attr(curr, "m0");
        current_M1[ 1] = get_real_attr(curr, "m1");
        current_M1[ 2] = get_real_attr(curr, "m2");
        current_M1[ 3] = get_real_attr(curr, "m3");
        current_M1[ 4] = get_real_attr(curr, "m4");
        current_M1[ 5] = get_real_attr(curr, "m5");
        current_M1[ 6] = get_real_attr(curr, "m6");
        current_M1[ 7] = get_real_attr(curr, "m7");
        current_M1[ 8] = get_real_attr(curr, "m8");
        current_M1[ 9] = get_real_attr(curr, "m9");
        current_M1[10] = get_real_attr(curr, "mA");
        current_M1[11] = get_real_attr(curr, "mB");
        current_M1[12] = get_real_attr(curr, "mC");
        current_M1[13] = get_real_attr(curr, "mD");
        current_M1[14] = get_real_attr(curr, "mE");
        current_M1[15] = get_real_attr(curr, "mF");
        current_a1     = get_real_attr(curr, "a");
        current_t1     = get_real_attr(curr, "t");
    }

    // If we're teleporting, just set all matrices.

    if (dt == 0)
    {
        load_mat(current_M0, current_M1);
        load_mat(current_M,  current_M1);
        load_inv(current_I,  current_M1);

        current_a0 = current_a1 = current_a;
        current_t0 = current_t1 = current_t;
    }
}

void app::user::goinit(double dt)
{
    // Advance to the first key, or begin again at the first.

    curr = root->child;

    gocurr(dt);
}

void app::user::gonext(double dt)
{
    // Advance to the next key, or begin again at the first.

    if (curr != 0)
        curr = mxmlWalkNext(curr, root, MXML_NO_DESCEND);
    if (curr == 0)
        curr = root->child;

    gocurr(dt);
}

void app::user::goprev(double dt)
{
    // Advance to the next key, or begin again at the first.

    if (curr != 0)
        curr = mxmlWalkPrev(curr, root, MXML_NO_DESCEND);
    if (curr == 0)
        curr = root->last_child;

    gocurr(dt);
}

void app::user::insert(double a, double t)
{
    mxml_node_t *node = mxmlNewElement(MXML_NO_PARENT, "key");

    set_real_attr(node, "m0", current_M[ 0]);
    set_real_attr(node, "m1", current_M[ 1]);
    set_real_attr(node, "m2", current_M[ 2]);
    set_real_attr(node, "m3", current_M[ 3]);
    set_real_attr(node, "m4", current_M[ 4]);
    set_real_attr(node, "m5", current_M[ 5]);
    set_real_attr(node, "m6", current_M[ 6]);
    set_real_attr(node, "m7", current_M[ 7]);
    set_real_attr(node, "m8", current_M[ 8]);
    set_real_attr(node, "m9", current_M[ 9]);
    set_real_attr(node, "mA", current_M[10]);
    set_real_attr(node, "mB", current_M[11]);
    set_real_attr(node, "mC", current_M[12]);
    set_real_attr(node, "mD", current_M[13]);
    set_real_attr(node, "mE", current_M[14]);
    set_real_attr(node, "mF", current_M[15]);
    set_real_attr(node, "a", a);
    set_real_attr(node, "t", t);

    mxmlAdd(root, MXML_ADD_AFTER, curr, node);

    current_a = a;
    current_t = t;
    gonext(1);

    dirty = true;
}

void app::user::remove()
{
    mxml_node_t *node = curr;

    gonext(1);

    mxmlDelete(node);

    dirty = true;
}

//-----------------------------------------------------------------------------

void app::user::slerp(const double *A,
                      const double *B,
                      const double *p, double t)
{
    double a[3];
    double b[3];
    double c[3];

    a[0] = A[12];
    a[1] = A[13];
    a[2] = A[14];

    b[0] = B[12];
    b[1] = B[13];
    b[2] = B[14];

    // Slerp the position about the given center.

    if (p)
    {
        a[0] -= p[0];
        a[1] -= p[1];
        a[2] -= p[2];

        b[0] -= p[0];
        b[1] -= p[1];
        b[2] -= p[2];

        ::slerp(c, a, b, t);

        c[0] += p[0];
        c[1] += p[1];
        c[2] += p[2];
    }
    else
    {
        ::slerp(c, a, b, t);
    }

    current_M[12] = c[0];
    current_M[13] = c[1];
    current_M[14] = c[2];
    current_M[15] = 1;

    // Slerp the rotation about the position.

    ::slerp(current_M + 0, A + 0, B + 0, t);
    ::slerp(current_M + 8, A + 8, B + 8, t);

    orthonormalize(current_M);

    load_inv(current_I, current_M);
}

//-----------------------------------------------------------------------------
