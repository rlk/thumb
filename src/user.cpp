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

app::user::user(int w, int h) :
    w(w), h(h), n(1), f(100), prog(0), type(type_mono), mode(mode_norm)
{
    const double a = double(w) / double(h);

    // Set some reasonable defaults.

    P[0] = 0; P[1] = 0; P[2] = 0;
    X[0] = 1; X[1] = 0; X[2] = 0;

    R[0] = 1; R[1] = 0; R[2] = 0;
    U[0] = 0; U[1] = 1; U[2] = 0;
    N[0] = 0; N[1] = 0; N[2] = 1;

    BL[0] = -0.5 * a; BL[1] = -0.5; BL[2] = -1.0;
    BR[0] = +0.5 * a; BR[1] = -0.5; BR[2] = -1.0;
    TL[0] = -0.5 * a; TL[1] = +0.5; TL[2] = -1.0;

    load_idt(default_M);
    load_idt(current_M);

    load_idt(curr_M0);
    load_idt(curr_M1);
 
    find_P();

    t0 = tt = t1 = 0;

    curr_a = curr_a0 = curr_a1 = 0;

    init();
    load();
    gonext(0);
}

app::user::~user()
{
//  if (prog) ::glob->free_program(prog);
    save();
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

void app::user::find_P()
{
    double O[16];
    double T[16];

    // Compute the extents of the frustum.

    double k = (N[0] * (BL[0] - P[0]) + 
                N[1] * (BL[1] - P[1]) +
                N[2] * (BL[2] - P[2]));

    double l = n * (R[0] * (P[0] - BL[0]) +
                    R[1] * (P[1] - BL[1]) +
                    R[2] * (P[2] - BL[2])) / k;
    double r = n * (R[0] * (P[0] - BR[0]) +
                    R[1] * (P[1] - BR[1]) +
                    R[2] * (P[2] - BR[2])) / k;
    double b = n * (U[0] * (P[0] - BL[0]) +
                    U[1] * (P[1] - BL[1]) +
                    U[2] * (P[2] - BL[2])) / k;
    double t = n * (U[0] * (P[0] - TL[0]) +
                    U[1] * (P[1] - TL[1]) +
                    U[2] * (P[2] - TL[2])) / k;

    // Generate the off-axis projection.

    load_persp(current_P, l, r, b, t, n, f);

    // Use the screen space basis to account for orientation.

    O[0] = R[0]; O[4] = R[1]; O[ 8] = R[2]; O[12] =  0.0;
    O[1] = U[0]; O[5] = U[1]; O[ 9] = U[2]; O[13] =  0.0;
    O[2] = N[0]; O[6] = N[1]; O[10] = N[2]; O[14] =  0.0;
    O[3] =  0.0; O[7] =  0.0; O[11] =  0.0; O[15] =  1.0;

    // Move the apex of the frustum to the origin.

    load_xlt_inv(T, P[0], P[1], P[2]);

    // Finally, compose the projection.

    mult_mat_mat(current_P, current_P, O);
    mult_mat_mat(current_P, current_P, T);
}

//-----------------------------------------------------------------------------

void app::user::clr()
{
    load_mat(current_M, default_M);
}

void app::user::get_M(double *M)
{
    load_mat(M, current_M);
}

void app::user::get_P(double *P)
{
    load_mat(P, current_P);
}

void app::user::set_M(const double *M)
{
    load_mat(current_M, M);
}

void app::user::set_P(const double *p)
{
    P[0] = p[0]; P[1] = p[1]; P[2] = p[2];

    find_P();
}

void app::user::set_V(const double *bl,
                      const double *br,
                      const double *tl,
                      const double *tr)
{
    // Store the frustum points.

    BL[0] = bl[0]; BL[1] = bl[1]; BL[2] = bl[2];
    BR[0] = br[0]; BR[1] = br[1]; BR[2] = br[2];
    TL[0] = tl[0]; TL[1] = tl[1]; TL[2] = tl[2];
    TR[0] = tr[0]; TR[1] = tr[1]; TR[2] = tr[2];

    // Find the basis of the screen space.

    R[0]  = BR[0] - BL[0];
    R[1]  = BR[1] - BL[1];
    R[2]  = BR[2] - BL[2];

    U[0]  = TL[0] - BL[0];
    U[1]  = TL[1] - BL[1];
    U[2]  = TL[2] - BL[2];

    N[0]  = R[1] * U[2] - R[2] * U[1];
    N[1]  = R[2] * U[0] - R[0] * U[2];
    N[2]  = R[0] * U[1] - R[1] * U[0];

    normalize(R);
    normalize(U);
    normalize(N);
}

//-----------------------------------------------------------------------------

void app::user::mult_S() const
{
    double T[16] = {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 0.5, 0.0,
        0.5, 0.5, 0.5, 1.0,
    };

    glMultMatrixd(T);
}

void app::user::mult_P() const
{
    glMultMatrixd(current_P);
}

void app::user::mult_O() const
{
    // Apply the orthogonal projection.

    glOrtho(0, w, h, 0, 0, 1);
}

void app::user::mult_M() const
{
    glMultMatrixd(current_M);
}

void app::user::mult_R() const
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

void app::user::mult_T() const
{
    // Apply the current view translation transform.

    glTranslated(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
}

void app::user::mult_V() const
{
    mult_R();
    mult_T();
}

//-----------------------------------------------------------------------------

void app::user::set_type(enum user_type t)
{
    // Free any existing program.

    if (prog) ::glob->free_program(prog);

    prog = 0;
    type = t;

    // Load the exposure program if necessary.

    if (t == type_varrier)  prog = ::glob->load_program("glsl/combiner.vert",
                                                        "glsl/combiner.frag");
    if (t == type_anaglyph) prog = ::glob->load_program("glsl/anaglyph.vert",
                                                        "glsl/anaglyph.frag");
    if (t == type_scanline) prog = ::glob->load_program("glsl/scanline.vert",
                                                        "glsl/scanline.frag");
    if (t == type_blended)  prog = ::glob->load_program("glsl/blended.vert",
                                                        "glsl/blended.frag");
}

void app::user::set_mode(enum user_mode m)
{
    mode = m;
}

//-----------------------------------------------------------------------------

void app::user::range(double N, double F)
{
    if (N < F)
    {
        n = N;
        f = F;
    }
    else
    {
        n =   0.1;
        f = 100.0;
    }

    find_P();
}

void app::user::draw() const
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

void app::user::push() const
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

void app::user::pop() const
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

void app::user::turn(double rx, double ry, double rz, double R[3][3])
{
    double M[16];
    double T[16];

    load_idt(M);

    M[ 0] = R[0][0]; M[ 1] = R[0][1]; M[ 2] = R[0][2];
    M[ 4] = R[1][0]; M[ 5] = R[1][1]; M[ 6] = R[1][2];
    M[ 8] = R[2][0]; M[ 9] = R[2][1]; M[10] = R[2][2];

    load_xps(T, M);

    mult_mat_mat(default_M, default_M, M);
    turn(rx, ry, rz);
    mult_mat_mat(default_M, default_M, T);

    clr();
}

void app::user::turn(double rx, double ry, double rz)
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

void app::user::move(double dx, double dy, double dz)
{
    Rmul_xlt_mat(default_M, dx, dy, dz);

    clr();
}

void app::user::home()
{
    load_idt(current_M);
    load_idt(default_M);
}

//-----------------------------------------------------------------------------
/*
void app::user::world_frustum(double *V) const
{
    // View plane.

    V[0] =  0;
    V[1] =  0;
    V[2] = -1;
    V[3] =  0;

    set_plane(V +  4, P, BL, TL);    // Left
    set_plane(V +  8, P, TR, BR);    // Right
    set_plane(V + 12, P, BR, BL);    // Bottom
    set_plane(V + 16, P, TL, TR);    // Top
}
*/
/*
void app::user::world_frustum(double *V) const
{
    const double A = double(w) / double(h);
    const double Z = 1.0;

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
    normalize(V + 16);
}
*/

void app::user::plane_frustum(double *V) const
{
    const double A = double(w) / double(h);
    const double Z = 1.0;

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

void app::user::point_frustum(double *V) const
{
    const double a = double(w) / double(h);

    const double *P = current_M + 12;
    const double *B = current_M +  8;

    double R[3];
    double U[3];

    double z = 1.0;

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
/*
void app::view::pick(double *p, double *v, int x, int y) const
{
    double z = 1.0;

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

    normalize(v);
}
*/
double app::user::dist(double *p) const
{
    return distance(p, current_M + 12);
}

//-----------------------------------------------------------------------------

bool app::user::step(double dt, const double *p, double& a)
{
    tt += dt;

    if (t0 <= tt && tt <= t1)
    {
        double k = (tt - t0) / (t1 - t0);
        double t = 3 * k * k - 2 * k * k * k;

        a = curr_a = (1 - t) * curr_a0 + t * curr_a1;

        slerp(curr_M0, curr_M1, p, t);

        return false;
    }
    else if (tt < t0)
    {
        load_mat(default_M, curr_M0);
        load_mat(current_M, curr_M0);
        a = curr_a = curr_a0;

        return false;
    }
    else if (tt > t1)
    {
        load_mat(default_M, curr_M1);
        load_mat(current_M, curr_M1);
        a = curr_a = curr_a1;

        return true;
    }

    return true;
}

void app::user::gocurr(double dt)
{
    t0 = tt;
    t1 = tt + dt;

    curr_a0 = curr_a;

    load_mat(curr_M0, current_M);
    load_mat(curr_M1, current_M);

    // Load the destination matrix from the XML element.

    if (curr)
    {
        curr_M1[ 0] = get_real_attr(curr, "m0");
        curr_M1[ 1] = get_real_attr(curr, "m1");
        curr_M1[ 2] = get_real_attr(curr, "m2");
        curr_M1[ 3] = get_real_attr(curr, "m3");
        curr_M1[ 4] = get_real_attr(curr, "m4");
        curr_M1[ 5] = get_real_attr(curr, "m5");
        curr_M1[ 6] = get_real_attr(curr, "m6");
        curr_M1[ 7] = get_real_attr(curr, "m7");
        curr_M1[ 8] = get_real_attr(curr, "m8");
        curr_M1[ 9] = get_real_attr(curr, "m9");
        curr_M1[10] = get_real_attr(curr, "mA");
        curr_M1[11] = get_real_attr(curr, "mB");
        curr_M1[12] = get_real_attr(curr, "mC");
        curr_M1[13] = get_real_attr(curr, "mD");
        curr_M1[14] = get_real_attr(curr, "mE");
        curr_M1[15] = get_real_attr(curr, "mF");
        curr_a1     = get_real_attr(curr, "a");
    }

    // If we're teleporting, just set all matrices.

    if (dt == 0)
    {
        load_mat(curr_M0,   curr_M1);
        load_mat(default_M, curr_M1);
        load_mat(current_M, curr_M1);

        curr_a0 = curr_a1 = curr_a;
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

void app::user::insert(double a)
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

    mxmlAdd(root, MXML_ADD_AFTER, curr, node);

    curr_a = a;
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

    default_M[12] = c[0];
    default_M[13] = c[1];
    default_M[14] = c[2];
    default_M[15] = 1;

    // Slerp the rotation about the position.

    ::slerp(default_M + 0, A + 0, B + 0, t);
    ::slerp(default_M + 8, A + 8, B + 8, t);

    // Orthonormalize.

    normalize(default_M + 0);
    normalize(default_M + 8);

    crossprod(default_M + 4, default_M + 8, default_M + 0);
    crossprod(default_M + 0, default_M + 4, default_M + 8);
    crossprod(default_M + 8, default_M + 0, default_M + 4);

    clr();
}

//-----------------------------------------------------------------------------

void app::user::sphere() const
{
    int i, r = 10, R = 90;
    int j, c = 10, C = 90;

    push();
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        mult_P();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPushAttrib(GL_ENABLE_BIT);
        {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);

            glColor3f(1.0f, 1.0f, 1.0f);

            // Draw lines of latitude.

            for (j = -90; j < 90; j += c)
            {
                if (j % C == 0)
                    glLineWidth(8.0);
                else
                    glLineWidth(2.0);

                glBegin(GL_LINE_LOOP);
                {
                    for (i = -180; i < 180; i += r)
                    {
                        double x = -sin(RAD(i)) * cos(RAD(j)) * n * 2;
                        double y =                sin(RAD(j)) * n * 2;
                        double z =  cos(RAD(i)) * cos(RAD(j)) * n * 2;

                        glVertex3d(x, y, z);
                    }
                }
                glEnd();
            }

            // Draw lines of longitude.

            for (i = -180; i < 180; i += r)
            {
                if (i % R == 0)
                    glLineWidth(8.0);
                else
                    glLineWidth(2.0);

                glBegin(GL_LINE_STRIP);
                {
                    for (j = -90; j < 90; j += c)
                    {
                        double x = -sin(RAD(i)) * cos(RAD(j)) * n * 2;
                        double y =                sin(RAD(j)) * n * 2;
                        double z =  cos(RAD(i)) * cos(RAD(j)) * n * 2;

                        glVertex3d(x, y, z);
                    }
                }
                glEnd();
            }
        }
        glPopAttrib();
    }
    pop();
}

//-----------------------------------------------------------------------------
