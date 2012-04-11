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

#include <GL/glew.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <limits>

#include "math3d.h"
#include "glsl.h"
#include "cube.hpp"

#include "scm-model.hpp"

//------------------------------------------------------------------------------

#if 0
typedef GLushort         GLindex;
#define GL_ELEMENT_INDEX GL_UNSIGNED_SHORT
#else
typedef GLuint           GLindex;
#define GL_ELEMENT_INDEX GL_UNSIGNED_INT
#endif

struct scm_model::page
{
    long long f;
    long long i;
    long long n;
    long long s;
    long long e;
    long long w;
    double    l;
    double    r;
    double    b;
    double    t;

    void set(long long i, long long n, long long s, long long e, long long w,
                          double    l, double    r, double    b, double    t)
    {
        assert(n >= 0);
        assert(s >= 0);
        assert(w >= 0);
        assert(e >= 0);

        this->f = 1;
        this->i = i;
        this->n = n;
        this->s = s;
        this->e = e;
        this->w = w;
        this->l = l;
        this->r = r;
        this->b = b;
        this->t = t;
    }
};

//------------------------------------------------------------------------------

static inline double scale(double k, double t)
{
    if (k < 1.0)
        return std::min(t / k, 1.0 - (1.0 - t) * k);
    else
        return std::max(t / k, 1.0 - (1.0 - t) * k);
}

void scm_model::zoom(double *w, const double *v)
{
    double d = vdot(v, zoomv);

    if (-1 < d && d < 1)
    {
        double b = scale(zoomk, acos(d) / M_PI) * M_PI;

        double y[3];
        double x[3];

        vcrs(y, v, zoomv);
        vnormalize(y, y);
        vcrs(x, zoomv, y);
        vnormalize(x, x);

        vmul(w, zoomv, cos(b));
        vmad(w, w,  x, sin(b));
    }
    else vcpy(w, v);
}

//------------------------------------------------------------------------------

static inline double length(const double *a, const double *b, int w, int h)
{
    if (a[3] <= 0 && b[3] <= 0) return 0;
    if (a[3] <= 0)              return HUGE_VAL;
    if (b[3] <= 0)              return HUGE_VAL;

    double dx = (a[0] / a[3] - b[0] / b[3]) * w / 2;
    double dy = (a[1] / a[3] - b[1] / b[3]) * h / 2;

    return sqrt(dx * dx + dy * dy);
}

static inline void cdiv(double *a)
{
    a[0] /= a[3];
    a[1] /= a[3];
    a[2] /= a[3];
}
#if 1

static inline void cmix(double *a, double *b, double k)
{
    a[0] = (1 - k) * a[0] + k * b[0];
    a[1] = (1 - k) * a[1] + k * b[1];
    a[2] = (1 - k) * a[2] + k * b[2];
    a[3] = (1 - k) * a[3] + k * b[3];
}

static inline void clip(double *a, double *b)
{
    const double e = 0.00001;  // Delicious fudge.

    while (1)
    {
        if      (a[0] < -a[3] && b[0] > -b[3])
            cmix(a, b, e + (a[3] + a[0]) / (a[3] + a[0] - b[3] - b[0]));

        else if (a[0] > +a[3] && b[0] < +b[3])
            cmix(a, b, e + (a[3] - a[0]) / (a[3] - a[0] - b[3] + b[0]));

        else if (a[1] < -a[3] && b[1] > -b[3])
            cmix(a, b, e + (a[3] + a[1]) / (a[3] + a[1] - b[3] - b[1]));

        else if (a[1] > +a[3] && b[1] < +b[3])
            cmix(a, b, e + (a[3] - a[1]) / (a[3] - a[1] - b[3] + b[1]));

        // else if (a[2] < -a[3] && b[2] > -b[3])
        //     cmix(a, b, e + (a[3] + a[2]) / (a[3] + a[2] - b[3] - b[2]));

        // else if (a[2] > +a[3] && b[2] < +b[3])
        //     cmix(a, b, e + (a[3] - a[2]) / (a[3] - a[2] - b[3] + b[2]));

        else break;
    }
}

#else
static inline void clip(double *a, double *b)
{
    const double xmin = -1.0;
    const double xmax = +1.0;
    const double ymin = -1.0;
    const double ymax = +1.0;
    const double zmin = -1.0;
    const double zmax = +1.0;

    while (1)
    {
        if      (a[2] < zmin && b[2] >= zmin)
        {
            a[0] = a[0] + (b[0] - a[0]) * (zmin - a[2]) / (b[2] - a[2]);
            a[1] = a[1] + (b[1] - a[1]) * (zmin - a[2]) / (b[2] - a[2]);
            a[2] = zmin;
        }
        else if (a[2] > zmax && b[2] <= zmax)
        {
            a[0] = a[0] + (b[0] - a[0]) * (zmax - a[2]) / (b[2] - a[2]);
            a[1] = a[1] + (b[1] - a[1]) * (zmax - a[2]) / (b[2] - a[2]);
            a[2] = zmax;
        }
        else if (a[1] < ymin && b[1] >= ymin)
        {
            a[0] = a[0] + (b[0] - a[0]) * (ymin - a[1]) / (b[1] - a[1]);
            a[2] = a[2] + (b[2] - a[2]) * (ymin - a[1]) / (b[1] - a[1]);
            a[1] = ymin;
        }
        else if (a[1] > ymax && b[1] <= ymax)
        {
            a[0] = a[0] + (b[0] - a[0]) * (ymax - a[1]) / (b[1] - a[1]);
            a[2] = a[2] + (b[2] - a[2]) * (ymax - a[1]) / (b[1] - a[1]);
            a[1] = ymax;
        }
        else if (a[0] < xmin && b[0] >= xmin)
        {
            a[1] = a[1] + (b[1] - a[1]) * (xmin - a[0]) / (b[0] - a[0]);
            a[2] = a[2] + (b[2] - a[2]) * (xmin - a[0]) / (b[0] - a[0]);
            a[0] = xmin;
        }
        else if (a[0] > xmax && b[0] <= xmax)
        {
            a[1] = a[1] + (b[1] - a[1]) * (xmax - a[0]) / (b[0] - a[0]);
            a[2] = a[2] + (b[2] - a[2]) * (xmax - a[0]) / (b[0] - a[0]);
            a[0] = xmax;
        }
        else break;
    }
}
#endif

static inline double clen(const double *a, const double *b, int w, int h)
{
    if (a[3] <= 0 && b[3] <= 0) return 0;
    if (a[3] <= 0)              return HUGE_VAL;
    if (b[3] <= 0)              return HUGE_VAL;

    double dx = (a[0] / a[3] - b[0] / b[3]) * w / 2;
    double dy = (a[1] / a[3] - b[1] / b[3]) * h / 2;

    return sqrt(dx * dx + dy * dy);
}

// TODO: Move this to where it belongs.

static void scube(long long f, double x, double y, double *v)
{
    const double s = x * M_PI_2 - M_PI_4;
    const double t = y * M_PI_2 - M_PI_4;

    double u[3];
    double w[3];

    u[0] =  sin(s) * cos(t);
    u[1] = -cos(s) * sin(t);
    u[2] =  cos(s) * cos(t);

    vnormalize(w, u);

    switch (f)
    {
        case 0: v[0] =  w[2]; v[1] =  w[1]; v[2] = -w[0]; break;
        case 1: v[0] = -w[2]; v[1] =  w[1]; v[2] =  w[0]; break;
        case 2: v[0] =  w[0]; v[1] =  w[2]; v[2] = -w[1]; break;
        case 3: v[0] =  w[0]; v[1] = -w[2]; v[2] =  w[1]; break;
        case 4: v[0] =  w[0]; v[1] =  w[1]; v[2] =  w[2]; break;
        case 5: v[0] = -w[0]; v[1] =  w[1]; v[2] = -w[2]; break;
    }
}

//------------------------------------------------------------------------------
#if 0
void norm4(double *v)
{
    double k = vlen(v);

    v[0] /= k;
    v[1] /= k;
    v[2] /= k;
    v[3] /= k;
}
#endif

double scm_model::wiew_face(const double *M, double rr, int vw, int vh,
                            double ee, double ww, double nn, double ss, int qi, int j)
{
    double ne[3], a[4], e[4], A[4], E[4];    // North-east corner
    double nw[3], b[4], f[4], B[4], F[4];    // North-west corner
    double se[3], c[4], g[4], C[4], G[4];    // South-east corner
    double sw[3], d[4], h[4], D[4], H[4];    // South-west corner

    scube(j, ee, nn, ne);
    scube(j, ww, nn, nw);
    scube(j, ee, ss, se);
    scube(j, ww, ss, sw);

    if (zoomk != 1)
    {
        zoom(ne, ne);
        zoom(nw, nw);
        zoom(se, se);
        zoom(sw, sw);
    }

    // Compute the maximum extent due to bulge.

    double v[3];

    v[0] = ne[0] + nw[0] + se[0] + sw[0];
    v[1] = ne[1] + nw[1] + se[1] + sw[1];
    v[2] = ne[2] + nw[2] + se[2] + sw[2];

    double r2 = r1 * vlen(v) / vdot(ne, v);

    // Apply the inner and outer radii to the bounding volume.

    vmul(a, ne, r0);
    vmul(b, nw, r0);
    vmul(c, se, r0);
    vmul(d, sw, r0);

    vmul(e, ne, r2);
    vmul(f, nw, r2);
    vmul(g, se, r2);
    vmul(h, sw, r2);

    // Compute W and reject any volume on the far side of the singularity.

    A[3] = M[ 3] * a[0] + M[ 7] * a[1] + M[11] * a[2] + M[15];
    B[3] = M[ 3] * b[0] + M[ 7] * b[1] + M[11] * b[2] + M[15];
    C[3] = M[ 3] * c[0] + M[ 7] * c[1] + M[11] * c[2] + M[15];
    D[3] = M[ 3] * d[0] + M[ 7] * d[1] + M[11] * d[2] + M[15];
    E[3] = M[ 3] * e[0] + M[ 7] * e[1] + M[11] * e[2] + M[15];
    F[3] = M[ 3] * f[0] + M[ 7] * f[1] + M[11] * f[2] + M[15];
    G[3] = M[ 3] * g[0] + M[ 7] * g[1] + M[11] * g[2] + M[15];
    H[3] = M[ 3] * h[0] + M[ 7] * h[1] + M[11] * h[2] + M[15];

    if (A[3] <= 0 && B[3] <= 0 && C[3] <= 0 && D[3] <= 0 &&
        E[3] <= 0 && F[3] <= 0 && G[3] <= 0 && H[3] <= 0)
        return 0;

    // Compute Z and trivially reject using the near and far clipping planes.

    A[2] = M[ 2] * a[0] + M[ 6] * a[1] + M[10] * a[2] + M[14];
    B[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14];
    C[2] = M[ 2] * c[0] + M[ 6] * c[1] + M[10] * c[2] + M[14];
    D[2] = M[ 2] * d[0] + M[ 6] * d[1] + M[10] * d[2] + M[14];
    E[2] = M[ 2] * e[0] + M[ 6] * e[1] + M[10] * e[2] + M[14];
    F[2] = M[ 2] * f[0] + M[ 6] * f[1] + M[10] * f[2] + M[14];
    G[2] = M[ 2] * g[0] + M[ 6] * g[1] + M[10] * g[2] + M[14];
    H[2] = M[ 2] * h[0] + M[ 6] * h[1] + M[10] * h[2] + M[14];

    if (A[2] >  A[3] && B[2] >  B[3] && C[2] >  C[3] && D[2] >  D[3] &&
        E[2] >  E[3] && F[2] >  F[3] && G[2] >  G[3] && H[2] >  H[3])
        return 0;
    if (A[2] < -A[3] && B[2] < -B[3] && C[2] < -C[3] && D[2] < -D[3] &&
        E[2] < -E[3] && F[2] < -F[3] && G[2] < -G[3] && H[2] < -H[3])
        return 0;

    // Compute Y and trivially reject using the bottom and top clipping planes.

    A[1] = M[ 1] * a[0] + M[ 5] * a[1] + M[ 9] * a[2] + M[13];
    B[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13];
    C[1] = M[ 1] * c[0] + M[ 5] * c[1] + M[ 9] * c[2] + M[13];
    D[1] = M[ 1] * d[0] + M[ 5] * d[1] + M[ 9] * d[2] + M[13];
    E[1] = M[ 1] * e[0] + M[ 5] * e[1] + M[ 9] * e[2] + M[13];
    F[1] = M[ 1] * f[0] + M[ 5] * f[1] + M[ 9] * f[2] + M[13];
    G[1] = M[ 1] * g[0] + M[ 5] * g[1] + M[ 9] * g[2] + M[13];
    H[1] = M[ 1] * h[0] + M[ 5] * h[1] + M[ 9] * h[2] + M[13];

    if (A[1] >  A[3] && B[1] >  B[3] && C[1] >  C[3] && D[1] >  D[3] &&
        E[1] >  E[3] && F[1] >  F[3] && G[1] >  G[3] && H[1] >  H[3])
        return 0;
    if (A[1] < -A[3] && B[1] < -B[3] && C[1] < -C[3] && D[1] < -D[3] &&
        E[1] < -E[3] && F[1] < -F[3] && G[1] < -G[3] && H[1] < -H[3])
        return 0;

    // Compute X and trivially reject using the left and right clipping planes.

    A[0] = M[ 0] * a[0] + M[ 4] * a[1] + M[ 8] * a[2] + M[12];
    B[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12];
    C[0] = M[ 0] * c[0] + M[ 4] * c[1] + M[ 8] * c[2] + M[12];
    D[0] = M[ 0] * d[0] + M[ 4] * d[1] + M[ 8] * d[2] + M[12];
    E[0] = M[ 0] * e[0] + M[ 4] * e[1] + M[ 8] * e[2] + M[12];
    F[0] = M[ 0] * f[0] + M[ 4] * f[1] + M[ 8] * f[2] + M[12];
    G[0] = M[ 0] * g[0] + M[ 4] * g[1] + M[ 8] * g[2] + M[12];
    H[0] = M[ 0] * h[0] + M[ 4] * h[1] + M[ 8] * h[2] + M[12];

    if (A[0] >  A[3] && B[0] >  B[3] && C[0] >  C[3] && D[0] >  D[3] &&
        E[0] >  E[3] && F[0] >  F[3] && G[0] >  G[3] && H[0] >  H[3])
        return 0;
    if (A[0] < -A[3] && B[0] < -B[3] && C[0] < -C[3] && D[0] < -D[3] &&
        E[0] < -E[3] && F[0] < -F[3] && G[0] < -G[3] && H[0] < -H[3])
        return 0;

    // Compute the length of the longest visible edge, in pixels.

    clip(A, E); clip(E, A);
    clip(B, F); clip(F, B);
    clip(C, G); clip(G, C);
    clip(D, H); clip(H, D);

    if (A[3] <= 0 || E[3] <= 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f); else glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex4dv(A); glVertex4dv(E);

    if (B[3] <= 0 || F[3] <= 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f); else glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex4dv(B); glVertex4dv(F);

    if (C[3] <= 0 || G[3] <= 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f); else glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex4dv(C); glVertex4dv(G);

    if (D[3] <= 0 || H[3] <= 0) glColor4f(1.0f, 0.0f, 0.0f, 0.5f); else glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glVertex4dv(D); glVertex4dv(H);

    return std::max(std::max(std::max(clen(A, B, vw, vh),
                                      clen(C, D, vw, vh)),
                             std::max(clen(A, C, vw, vh),
                                      clen(B, D, vw, vh))),
                    std::max(std::max(clen(E, F, vw, vh),
                                      clen(G, H, vw, vh)),
                             std::max(clen(E, G, vw, vh),
                                      clen(F, H, vw, vh))));
}

#define X -1

static const int wrap[6][6][4] = {
    { // 0
        { X, X, X, X }, // 0
        { X, X, X, X }, // 1
        { X, X, 0, 2 }, // 2
        { 2, 0, X, X }, // 3
        { 0, X, 2, X }, // 4
        { X, 1, X, 3 }, // 5
    },
    { // 1
        { X, X, X, X }, // 0
        { X, X, X, X }, // 1
        { X, X, 3, 1 }, // 2
        { 1, 3, X, X }, // 3
        { X, 1, X, 3 }, // 4
        { 0, X, 2, X }, // 5
    },
    { // 2
        { X, 0, X, 1 }, // 0
        { 1, X, 0, X }, // 1
        { X, X, X, X }, // 2
        { X, X, X, X }, // 3
        { 0, 1, X, X }, // 4
        { X, X, 1, 0 }, // 5
    },
    { // 3
        { X, 3, X, 2 }, // 0
        { 2, X, 3, X }, // 1
        { X, X, X, X }, // 2
        { X, X, X, X }, // 3
        { X, X, 2, 3 }, // 4
        { 3, 2, X, X }, // 5
    },
    { // 4
        { X, 1, X, 3 }, // 0
        { 0, X, 2, X }, // 1
        { X, X, 2, 3 }, // 2
        { 0, 1, X, X }, // 3
        { X, X, X, X }, // 4
        { X, X, X, X }, // 5
    },
    { // 5
        { 0, X, 2, X }, // 0
        { X, 1, X, 3 }, // 1
        { X, X, 1, 0 }, // 2
        { 3, 2, X, X }, // 3
        { X, X, X, X }, // 4
        { X, X, X, X }, // 5
    },
};

#undef X

void scm_model::pwep_face(const double *M, double R, int ww, int wh, int qi)
{
    const long long i = qv[qi].i;
    const long long n = qv[qi].n;
    const long long s = qv[qi].s;
    const long long e = qv[qi].e;
    const long long w = qv[qi].w;
    const double    l = qv[qi].l;
    const double    r = qv[qi].r;
    const double    b = qv[qi].b;
    const double    t = qv[qi].t;

    double k = wiew_face(M, R, ww, wh, r, l, t, b, qi, face_root(i));

    if (k > size && qn + 4 < qm && face_level(i) < 15)
    {
        const double x = (r + l) / 2;
        const double y = (t + b) / 2;

        const long long I = face_root(i);
        const long long N = face_root(n);
        const long long S = face_root(s);
        const long long E = face_root(e);
        const long long W = face_root(w);

        qv[qn++].set(face_child(i, 0),
                     face_child(n, wrap[I][N][2]),
                     face_child(i, 2),
                     face_child(e, wrap[I][E][1]),
                     face_child(i, 1),             x, r, y, t);
#if 1
        qv[qn++].set(face_child(i, 1),
                     face_child(n, wrap[I][N][3]),
                     face_child(i, 3),
                     face_child(i, 0),
                     face_child(w, wrap[I][W][0]), l, x, y, t);
        qv[qn++].set(face_child(i, 2),
                     face_child(i, 0),
                     face_child(s, wrap[I][S][0]),
                     face_child(e, wrap[I][E][3]),
                     face_child(i, 3),             x, r, b, y);
        qv[qn++].set(face_child(i, 3),
                     face_child(i, 1),
                     face_child(s, wrap[I][S][1]),
                     face_child(i, 2),
                     face_child(w, wrap[I][W][2]), l, x, b, y);
#endif
        qv[qi].f = 0;
    }
}

void scm_model::pwep(const double *PP, const double *VV, int w, int h)
{
    double M[16];
    double I[16];
    double u[4] = { 0.0,  0.0, -1.0,  1.0 };
    double v[4];

    double P[16];
    double V[16];

    glGetDoublev(GL_PROJECTION_MATRIX, P);
    glGetDoublev(GL_MODELVIEW_MATRIX, V);

    mmultiply(M, P, V);
    minvert(I, M);
    wtransform(v, I, u);
    v[0] /= v[3];
    v[1] /= v[3];
    v[2] /= v[3];
    v[3] /= v[3];
    double r = vlen(v);

    qn = 0;

    qv[qn++].set(0, 2, 3, 5, 4, 1, 0, 1, 0);
#if 0
    qv[qn++].set(1, 2, 3, 4, 5, 1, 0, 1, 0);
    qv[qn++].set(2, 5, 4, 0, 1, 1, 0, 1, 0);
    qv[qn++].set(3, 4, 5, 1, 0, 1, 0, 1, 0);
    qv[qn++].set(4, 2, 3, 0, 1, 1, 0, 1, 0);
    qv[qn++].set(5, 2, 3, 1, 0, 1, 0, 1, 0);
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glUseProgram(0);
    glBegin(GL_LINES);

    for (int qi = 0; qi < qn; ++qi)
        pwep_face(M, r, w, h, qi);

    glEnd();

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);
}

void scm_model::dwaw(const double *P, const double *V, int w, int h,
                                                      const int *vv, int vc,
                                                      const int *fv, int fc,
                                                      const int *pv, int pc)
{
    // glMatrixMode(GL_PROJECTION);
    // glLoadMatrixd(P);
    // glMatrixMode(GL_MODELVIEW);
    // glLoadMatrixd(V);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    pwep(P, V, w, h);

#if 0
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glUseProgram(program);
    {
        glUniform1f(u_zoomk, GLfloat(zoomk));
        glUniform3f(u_zoomv, GLfloat(zoomv[0]),
                             GLfloat(zoomv[1]),
                             GLfloat(zoomv[2]));

        for (int qi = 0; qi < qn; ++qi)
        {
            const long long l = face_level(qv[qi].i);
            const long long f = face_root (qv[qi].i);

            glUniform2f(u_tex_a[l], GLfloat(qv[qi].r), GLfloat(qv[qi].t));
            glUniform2f(u_tex_d[l], GLfloat(qv[qi].l), GLfloat(qv[qi].b));

            if (qv[qi].f)
            {
                static const GLfloat faceM[6][9] = {
                    {  0.f,  0.f,  1.f,  0.f,  1.f,  0.f, -1.f,  0.f,  0.f },
                    {  0.f,  0.f, -1.f,  0.f,  1.f,  0.f,  1.f,  0.f,  0.f },
                    {  1.f,  0.f,  0.f,  0.f,  0.f,  1.f,  0.f, -1.f,  0.f },
                    {  1.f,  0.f,  0.f,  0.f,  0.f, -1.f,  0.f,  1.f,  0.f },
                    {  1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f,  1.f },
                    { -1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f, -1.f },
                };

                glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[f]);

                glUniform1i(u_level, l);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[0]);
                glDrawElements(GL_QUADS, count, GL_ELEMENT_INDEX, 0);
            }
        }
    }
    glUseProgram(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

//------------------------------------------------------------------------------

scm_model::scm_model(scm_cache& cache,
                     const char *vert,
                     const char *frag,
                     int n, int s, int d, double r0, double r1) :
    cache(cache),
    time(1),
    size(s),
    depth(d),
    r0(r0),
    r1(r1),
    status(cube_size(d), s_halt)
{
    init_program(vert, frag);
    init_arrays(n);

    zoomv[0] =  0;
    zoomv[1] =  0;
    zoomv[2] = -1;
    zoomk    =  1;

    qm = 8192;
    qn =    0;
    qv = (page *) calloc(qm, sizeof (page));
}

scm_model::~scm_model()
{
    free(qv);

    free_arrays();
    free_program();
}

//------------------------------------------------------------------------------

// TODO: reorder the arguments LRTB.

double scm_model::view_face(const double *M, double rr, int vw, int vh,
                            double ee, double ww, double nn, double ss, int j)
{
    double ne[3], a[3], e[3], A[4], E[4];    // North-east corner
    double nw[3], b[3], f[3], B[4], F[4];    // North-west corner
    double se[3], c[3], g[3], C[4], G[4];    // South-east corner
    double sw[3], d[3], h[3], D[4], H[4];    // South-west corner

    scube(j, ee, nn, ne);
    scube(j, ww, nn, nw);
    scube(j, ee, ss, se);
    scube(j, ww, ss, sw);

    if (zoomk != 1)
    {
        zoom(ne, ne);
        zoom(nw, nw);
        zoom(se, se);
        zoom(sw, sw);
    }

    // Compute the maximum extent due to bulge.

    double v[3];

    v[0] = ne[0] + nw[0] + se[0] + sw[0];
    v[1] = ne[1] + nw[1] + se[1] + sw[1];
    v[2] = ne[2] + nw[2] + se[2] + sw[2];

    double k = vlen(v) / vdot(ne, v);

    // Apply the inner and outer radii to the bounding volume.

    vmul(a, ne, r0);
    vmul(b, nw, r0);
    vmul(c, se, r0);
    vmul(d, sw, r0);

    vmul(e, ne, r1 * k);
    vmul(f, nw, r1 * k);
    vmul(g, se, r1 * k);
    vmul(h, sw, r1 * k);

    // Compute W and reject any volume on the far side of the singularity.

    A[3] = M[ 3] * a[0] + M[ 7] * a[1] + M[11] * a[2] + M[15];
    B[3] = M[ 3] * b[0] + M[ 7] * b[1] + M[11] * b[2] + M[15];
    C[3] = M[ 3] * c[0] + M[ 7] * c[1] + M[11] * c[2] + M[15];
    D[3] = M[ 3] * d[0] + M[ 7] * d[1] + M[11] * d[2] + M[15];
    E[3] = M[ 3] * e[0] + M[ 7] * e[1] + M[11] * e[2] + M[15];
    F[3] = M[ 3] * f[0] + M[ 7] * f[1] + M[11] * f[2] + M[15];
    G[3] = M[ 3] * g[0] + M[ 7] * g[1] + M[11] * g[2] + M[15];
    H[3] = M[ 3] * h[0] + M[ 7] * h[1] + M[11] * h[2] + M[15];

    if (A[3] <= 0 && B[3] <= 0 && C[3] <= 0 && D[3] <= 0 &&
        E[3] <= 0 && F[3] <= 0 && G[3] <= 0 && H[3] <= 0)
        return 0;

    // Compute Z and apply the near and far clipping planes.

    A[2] = M[ 2] * a[0] + M[ 6] * a[1] + M[10] * a[2] + M[14];
    B[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14];
    C[2] = M[ 2] * c[0] + M[ 6] * c[1] + M[10] * c[2] + M[14];
    D[2] = M[ 2] * d[0] + M[ 6] * d[1] + M[10] * d[2] + M[14];
    E[2] = M[ 2] * e[0] + M[ 6] * e[1] + M[10] * e[2] + M[14];
    F[2] = M[ 2] * f[0] + M[ 6] * f[1] + M[10] * f[2] + M[14];
    G[2] = M[ 2] * g[0] + M[ 6] * g[1] + M[10] * g[2] + M[14];
    H[2] = M[ 2] * h[0] + M[ 6] * h[1] + M[10] * h[2] + M[14];

    if (A[2] >  A[3] && B[2] >  B[3] && C[2] >  C[3] && D[2] >  D[3] &&
        E[2] >  E[3] && F[2] >  F[3] && G[2] >  G[3] && H[2] >  H[3])
        return 0;
    if (A[2] < -A[3] && B[2] < -B[3] && C[2] < -C[3] && D[2] < -D[3] &&
        E[2] < -E[3] && F[2] < -F[3] && G[2] < -G[3] && H[2] < -H[3])
        return 0;

    // Compute Y and apply the bottom and top clipping planes.

    A[1] = M[ 1] * a[0] + M[ 5] * a[1] + M[ 9] * a[2] + M[13];
    B[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13];
    C[1] = M[ 1] * c[0] + M[ 5] * c[1] + M[ 9] * c[2] + M[13];
    D[1] = M[ 1] * d[0] + M[ 5] * d[1] + M[ 9] * d[2] + M[13];
    E[1] = M[ 1] * e[0] + M[ 5] * e[1] + M[ 9] * e[2] + M[13];
    F[1] = M[ 1] * f[0] + M[ 5] * f[1] + M[ 9] * f[2] + M[13];
    G[1] = M[ 1] * g[0] + M[ 5] * g[1] + M[ 9] * g[2] + M[13];
    H[1] = M[ 1] * h[0] + M[ 5] * h[1] + M[ 9] * h[2] + M[13];

    if (A[1] >  A[3] && B[1] >  B[3] && C[1] >  C[3] && D[1] >  D[3] &&
        E[1] >  E[3] && F[1] >  F[3] && G[1] >  G[3] && H[1] >  H[3])
        return 0;
    if (A[1] < -A[3] && B[1] < -B[3] && C[1] < -C[3] && D[1] < -D[3] &&
        E[1] < -E[3] && F[1] < -F[3] && G[1] < -G[3] && H[1] < -H[3])
        return 0;

    // Compute X and apply the left and right clipping planes.

    A[0] = M[ 0] * a[0] + M[ 4] * a[1] + M[ 8] * a[2] + M[12];
    B[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12];
    C[0] = M[ 0] * c[0] + M[ 4] * c[1] + M[ 8] * c[2] + M[12];
    D[0] = M[ 0] * d[0] + M[ 4] * d[1] + M[ 8] * d[2] + M[12];
    E[0] = M[ 0] * e[0] + M[ 4] * e[1] + M[ 8] * e[2] + M[12];
    F[0] = M[ 0] * f[0] + M[ 4] * f[1] + M[ 8] * f[2] + M[12];
    G[0] = M[ 0] * g[0] + M[ 4] * g[1] + M[ 8] * g[2] + M[12];
    H[0] = M[ 0] * h[0] + M[ 4] * h[1] + M[ 8] * h[2] + M[12];

    if (A[0] >  A[3] && B[0] >  B[3] && C[0] >  C[3] && D[0] >  D[3] &&
        E[0] >  E[3] && F[0] >  F[3] && G[0] >  G[3] && H[0] >  H[3])
        return 0;
    if (A[0] < -A[3] && B[0] < -B[3] && C[0] < -C[3] && D[0] < -D[3] &&
        E[0] < -E[3] && F[0] < -F[3] && G[0] < -G[3] && H[0] < -H[3])
        return 0;

    // Compute the length of the longest visible edge, in pixels.


    rr = std::max(rr, r0);
    rr = std::min(rr, r1);

    A[0] = ne[0] * rr; A[1] = ne[1] * rr; A[2] = ne[2] * rr; A[3] = 1.0;
    B[0] = nw[0] * rr; B[1] = nw[1] * rr; B[2] = nw[2] * rr; B[3] = 1.0;
    C[0] = se[0] * rr; C[1] = se[1] * rr; C[2] = se[2] * rr; C[3] = 1.0;
    D[0] = sw[0] * rr; D[1] = sw[1] * rr; D[2] = sw[2] * rr; D[3] = 1.0;

    wtransform(E, M, A);
    wtransform(F, M, B);
    wtransform(G, M, C);
    wtransform(H, M, D);

    return std::max(std::max(length(E, F, vw, vh),
                             length(G, H, vw, vh)),
                    std::max(length(E, G, vw, vh),
                             length(F, H, vw, vh)));
}

void scm_model::dump_face(const double *M, double rr, int vw, int vh,
                            double ee, double ww, double nn, double ss, int j)
{
    double ne[3], A[4], E[4];    // North-east corner
    double nw[3], B[4], F[4];    // North-west corner
    double se[3], C[4], G[4];    // South-east corner
    double sw[3], D[4], H[4];    // South-west corner

    scube(j, ee, nn, ne);
    scube(j, ww, nn, nw);
    scube(j, ee, ss, se);
    scube(j, ww, ss, sw);

    if (zoomk != 1)
    {
        zoom(ne, ne);
        zoom(nw, nw);
        zoom(se, se);
        zoom(sw, sw);
    }

    rr = 1.0;

    A[0] = ne[0] * rr; A[1] = ne[1] * rr; A[2] = ne[2] * rr; A[3] = 1.0;
    B[0] = nw[0] * rr; B[1] = nw[1] * rr; B[2] = nw[2] * rr; B[3] = 1.0;
    C[0] = se[0] * rr; C[1] = se[1] * rr; C[2] = se[2] * rr; C[3] = 1.0;
    D[0] = sw[0] * rr; D[1] = sw[1] * rr; D[2] = sw[2] * rr; D[3] = 1.0;

    glVertex4dv(A); glVertex4dv(B);
    glVertex4dv(A); glVertex4dv(C);
    glVertex4dv(B); glVertex4dv(D);
    glVertex4dv(C); glVertex4dv(D);

    // if (ee > 0 || ww > 0) return;

    A[0] = ne[0] * r0; A[1] = ne[1] * r0; A[2] = ne[2] * r0; A[3] = 1.0;
    B[0] = nw[0] * r0; B[1] = nw[1] * r0; B[2] = nw[2] * r0; B[3] = 1.0;
    C[0] = se[0] * r0; C[1] = se[1] * r0; C[2] = se[2] * r0; C[3] = 1.0;
    D[0] = sw[0] * r0; D[1] = sw[1] * r0; D[2] = sw[2] * r0; D[3] = 1.0;

    E[0] = ne[0] * r1; E[1] = ne[1] * r1; E[2] = ne[2] * r1; E[3] = 1.0;
    F[0] = nw[0] * r1; F[1] = nw[1] * r1; F[2] = nw[2] * r1; F[3] = 1.0;
    G[0] = se[0] * r1; G[1] = se[1] * r1; G[2] = se[2] * r1; G[3] = 1.0;
    H[0] = sw[0] * r1; H[1] = sw[1] * r1; H[2] = sw[2] * r1; H[3] = 1.0;

    glVertex4dv(A); glVertex4dv(E);
    glVertex4dv(B); glVertex4dv(F);
    glVertex4dv(C); glVertex4dv(G);
    glVertex4dv(D); glVertex4dv(H);
}

//------------------------------------------------------------------------------

void scm_model::prep(const double *P, const double *V, int w, int h)
{
    double M[16];

    mmultiply(M, P, V);

    for (int i = 0; i < 6; ++i)
        prep_face(M, w, h, 0, 1, 0, 1, i, 0, i);
}

#if 1

void scm_model::prep_face(const double *M, int w, int h,
                          double r, double l,
                          double t, double b, int j, int d, int i)
{
    if (d < depth)
    {
        double s = view_face(M, 0, w, h, r, l, t, b, j);

        if (s > 0)
        {
            const int i0 = face_child(i, 0);
            const int i1 = face_child(i, 1);
            const int i2 = face_child(i, 2);
            const int i3 = face_child(i, 3);

            if (s < size)
            {
                status[i]  = s_draw;
                status[i0] = s_halt;
                status[i1] = s_halt;
                status[i2] = s_halt;
                status[i3] = s_halt;
            }
            else
            {
                const double x = (r + l) * 0.5;
                const double y = (t + b) * 0.5;

                prep_face(M, w, h, r, x, t, y, j, d + 1, i0);
                prep_face(M, w, h, x, l, t, y, j, d + 1, i1);
                prep_face(M, w, h, r, x, y, b, j, d + 1, i2);
                prep_face(M, w, h, x, l, y, b, j, d + 1, i3);

                if (status[i0] == s_halt &&
                    status[i1] == s_halt &&
                    status[i2] == s_halt &&
                    status[i3] == s_halt)

                    status[i] = s_halt;
                else
                    status[i] = s_pass;
            }
        }
        else
            status[i] = s_halt;
    }
    else
    {
        double s = view_face(M, 0, w, h, r, l, t, b, j);

        if (s > 0)
            status[i] = s_draw;
        else
            status[i] = s_halt;
    }
}

#else

void scm_model::prep_face(const double *M, int w, int h,
                          double r, double l,
                          double t, double b, int j, int d, int i)

{
    const int i0 = face_child(i, 0);
    const int i1 = face_child(i, 1);
    const int i2 = face_child(i, 2);
    const int i3 = face_child(i, 3);

    if (d < depth)
    {
        const double x = (r + l) * 0.5;
        const double y = (t + b) * 0.5;

        prep_face(M, w, h, r, x, t, y, j, d + 1, i0);
        prep_face(M, w, h, x, l, t, y, j, d + 1, i1);
        prep_face(M, w, h, r, x, y, b, j, d + 1, i2);
        prep_face(M, w, h, x, l, y, b, j, d + 1, i3);
    }

    double s = view_face(M, w, h, r, l, t, b, j);

    if (d < depth)
    {
        if (0 < s && s < size)
        {
            status[i]  = s_draw;
            status[i0] = s_halt;
            status[i1] = s_halt;
            status[i2] = s_halt;
            status[i3] = s_halt;
        }
        else
        {
            if (status[i0] == s_halt &&
                status[i1] == s_halt &&
                status[i2] == s_halt &&
                status[i3] == s_halt)

                status[i] = s_halt;
            else
                status[i] = s_pass;
        }
    }
    else
    {
        if (0 < s)
            status[i] = s_draw;
        else
            status[i] = s_halt;
    }
}

#endif
//------------------------------------------------------------------------------

GLfloat scm_model::age(int then)
{
    GLfloat a = GLfloat(time - then) / 60.f;
    return (a > 1.f) ? 1.f : a;
}

void scm_model::draw(const double *P, const double *V, const int *vv, int vc,
                                                       const int *fv, int fc,
                                                       const int *pv, int pc)
{
    double M[16];

    mmultiply(M, P, V);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    for (int i = 15; i >= 0; --i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, cache.get_fill());
    }

    // This is a hack that ensures that the root pages of all files are touched.

    GLuint o;
    int tock;

    for (int i = 0; i < vc; ++i)
        for (int j = 0; j < 6; ++j)
            o = cache.get_page(vv[i], j, time, tock);

    for (int i = 0; i < fc; ++i)
        for (int j = 0; j < 6; ++j)
            o = cache.get_page(fv[i], j, time, tock);

    for (int i = 0; i < pc; ++i)
        for (int j = 0; j < 6; ++j)
            o = cache.get_page(pv[i], j, time, tock);

    glUseProgram(program);
    {
        glUniform1f(u_zoomk, GLfloat(zoomk));
        glUniform3f(u_zoomv, GLfloat(zoomv[0]),
							 GLfloat(zoomv[1]),
							 GLfloat(zoomv[2]));

        for (int i = 0; i < 6; ++i)
        {
            static const GLfloat faceM[6][9] = {
                {  0.f,  0.f,  1.f,  0.f,  1.f,  0.f, -1.f,  0.f,  0.f },
                {  0.f,  0.f, -1.f,  0.f,  1.f,  0.f,  1.f,  0.f,  0.f },
                {  1.f,  0.f,  0.f,  0.f,  0.f,  1.f,  0.f, -1.f,  0.f },
                {  1.f,  0.f,  0.f,  0.f,  0.f, -1.f,  0.f,  1.f,  0.f },
                {  1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f,  1.f },
                { -1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f, -1.f },
            };

            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[i]);

            draw_face(vv, vc, fv, fc, pv, pc, 0, 1, 0, 1, 0, i);
        }
    }
    glUseProgram(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glDisableClientState(GL_VERTEX_ARRAY);

    glActiveTexture(GL_TEXTURE0);
}

void scm_model::draw_face(const int *vv, int vc,
                          const int *fv, int fc,
                          const int *pv, int pc,
                          double r, double l, double t, double b, int d, int i)
{
    int then = time;

    // If this page does NOT have the HALT state then we're either drawing it
    // or drawing one of its descendents. Either way, bind the vert and frag
    // images and set their ages.

    if (status[i] != s_halt)
    {
        // Vertex shader images and ages.

        for (int vi = 0; vi < vc; ++vi)
        {
            glActiveTexture(GL_TEXTURE0 + 8 * vi + d);
            glBindTexture(GL_TEXTURE_2D, cache.get_page(vv[vi], i, time, then));
            glUniform1f(u_v_age[8 * vi + d], age(then));
        }

        // Fragment shader images and ages.

        for (int fi = 0; fi < fc; ++fi)
        {
            glActiveTexture(GL_TEXTURE0 + 8 * fi + d + 8);
            glBindTexture(GL_TEXTURE_2D, cache.get_page(fv[fi], i, time, then));
            glUniform1f(u_f_age[8 * fi + d], age(then));
        }

        // Page coordinates.

        glUniform2f(u_tex_a[d], GLfloat(r), GLfloat(t));
        glUniform2f(u_tex_d[d], GLfloat(l), GLfloat(b));
    }

    // If this page has the PASS state then we know it represents an initial
    // subset of the active page set. It's a good candidate for precaching
    // in any data sets we think we might need soon.

    if (status[i] == s_pass)
    {
        for (int pi = 0; pi < pc; ++pi)
            cache.get_page(pv[pi], i, time, then);
    }

    // If this page has the PASS state then draw the descendents.

    if (status[i] == s_pass)
    {
        const double x = (r + l) * 0.5;
        const double y = (t + b) * 0.5;

        draw_face(vv, vc, fv, fc, pv, pc, r, x, t, y, d + 1, face_child(i, 0));
        draw_face(vv, vc, fv, fc, pv, pc, x, l, t, y, d + 1, face_child(i, 1));
        draw_face(vv, vc, fv, fc, pv, pc, r, x, y, b, d + 1, face_child(i, 2));
        draw_face(vv, vc, fv, fc, pv, pc, x, l, y, b, d + 1, face_child(i, 3));
    }

    // If this page has the DRAW state then do draw it. Begin by determining
    // whether any of its neighbors will be also be drawn and select an element
    // winding accordingly.

    if (status[i] == s_draw)
    {
        long long n, s, e, w, j = 0;

        face_neighbors(i, n, s, e, w);

        if (i > 5) j = (status[face_parent(n)] == s_draw ? 1 : 0)
                     | (status[face_parent(s)] == s_draw ? 2 : 0)
                     | (status[face_parent(e)] == s_draw ? 4 : 0)
                     | (status[face_parent(w)] == s_draw ? 8 : 0);

        glUniform1i(u_level, d);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[j]);
        glDrawElements(GL_QUADS, count, GL_ELEMENT_INDEX, 0);
    }

    // If this page does NOT have the HALT state then we must have set some
    // texture bindings above. Undo them.

    if (status[i] != s_halt)
    {
        for (int vi = 0; vi < vc; ++vi)
        {
            glActiveTexture(GL_TEXTURE0 + 8 * vi + d);
            glBindTexture(GL_TEXTURE_2D, cache.get_fill());
        }
        for (int fi = 0; fi < fc; ++fi)
        {
            glActiveTexture(GL_TEXTURE0 + 8 * fi + d + 8);
            glBindTexture(GL_TEXTURE_2D, cache.get_fill());
        }
    }
}

//------------------------------------------------------------------------------

void scm_model::set_fade(double k)
{
    double t = k * k * (3.0 - 2.0 * k);
    glUseProgram(program);
    glUniform1f(u_fader, GLfloat(t));
}

static GLuint glGetUniformLocationv(GLuint program, const char *fmt, int d)
{
    char str[256];
    sprintf(str, fmt, d);
    return glGetUniformLocation(program, str);
}

void scm_model::init_program(const char *vert_src,
                             const char *frag_src)
{
    if (vert_src && frag_src)
    {
        vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_src);
        frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_src);
        program     = glsl_init_program(vert_shader, frag_shader);

        glUseProgram(program);

        u_level = glGetUniformLocation(program, "level");
        u_fader = glGetUniformLocation(program, "fader");
        u_zoomk = glGetUniformLocation(program, "zoomk");
        u_zoomv = glGetUniformLocation(program, "zoomv");
        u_faceM = glGetUniformLocation(program, "faceM");

        for (int d = 0; d < 16; ++d)
        {
            u_tex_a[d] = glGetUniformLocationv(program, "tex_a[%d]", d);
            u_tex_d[d] = glGetUniformLocationv(program, "tex_d[%d]", d);
            u_v_age[d] = glGetUniformLocationv(program, "v_age[%d]", d);
            u_f_age[d] = glGetUniformLocationv(program, "f_age[%d]", d);
            // glUniform1i (glGetUniformLocationv(program, "v_img[%d]", d), d);
            // glUniform1i (glGetUniformLocationv(program, "f_img[%d]", d), d + 8);
            // glUniform1i (glGetUniformLocationv(program, "f_img[%d]", d+8), d + 16);
        }
    }
}

void scm_model::free_program()
{
    glDeleteProgram(program);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);
}

//------------------------------------------------------------------------------

static inline int up(int d)
{
    return (d % 2) ? d + 1 : d;
}

static inline int dn(int d)
{
    return (d % 2) ? d - 1 : d;
}

static void init_vertices(int n)
{
    struct vertex
    {
        GLfloat x;
        GLfloat y;
    };

    const size_t s = (n + 1) * (n + 1) * sizeof (vertex);

    if (vertex *p = (vertex *) malloc(s))
    {
        vertex *v = p;

        // Compute the position of each vertex.

        for     (int r = 0; r <= n; ++r)
            for (int c = 0; c <= n; ++c, ++v)
            {
                v->x = GLfloat(c) / GLfloat(n);
                v->y = GLfloat(r) / GLfloat(n);
            }

        // Upload the vertices to the vertex buffer.

        glBufferData(GL_ARRAY_BUFFER, s, p, GL_STATIC_DRAW);
        free(p);
    }
}

static void init_elements(int n, int b)
{
    struct element
    {
        GLindex a;
        GLindex b;
        GLindex d;
        GLindex c;
    };

    const size_t s = n * n * sizeof (element);
    const int    d = n + 1;

    if (element *p = (element *) malloc(s))
    {
        element *e = p;

        // Compute the indices for each quad.

        for     (int r = 0; r < n; ++r)
            for (int c = 0; c < n; ++c, ++e)
            {
                e->a = GLindex(d * (r    ) + (c    ));
                e->b = GLindex(d * (r    ) + (c + 1));
                e->c = GLindex(d * (r + 1) + (c    ));
                e->d = GLindex(d * (r + 1) + (c + 1));
            }

        // Rewind the indices to reduce edge resolution as necessary.

        element *N = p;
        element *W = p + (n - 1);
        element *E = p;
        element *S = p + (n - 1) * n;

        for (int i = 0; i < n; ++i, N += 1, S += 1, E += n, W += n)
        {
            if (b & 1) { if (i & 1) N->a -= 1; else N->b -= 1; }
            if (b & 2) { if (i & 1) S->c += 1; else S->d += 1; }
            if (b & 4) { if (i & 1) E->a += d; else E->c += d; }
            if (b & 8) { if (i & 1) W->b -= d; else W->d -= d; }
        }

        // Upload the indices to the element buffer.

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s, p, GL_STATIC_DRAW);
        free(p);
    }
}

void scm_model::init_arrays(int n)
{
    glGenBuffers(1, &vertices);
    glGenBuffers(16, elements);

    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    init_vertices(n);

    for (int b = 0; b < 16; ++b)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[b]);
        init_elements(n, b);
    }

    count = 4 * n * n;
}

void scm_model::free_arrays()
{
    glDeleteBuffers(16, elements);
    glDeleteBuffers(1, &vertices);
}

//------------------------------------------------------------------------------
