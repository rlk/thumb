//  Copyright (C) 2007 Robert Kooima
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

#include "etc-math.hpp"
#include "ogl-opengl.hpp"
#include "uni-spatch.hpp"

//-----------------------------------------------------------------------------

static void subdiv(const double a[3], const double e[3], double r)
{
    double b[3];
    double c[3];
    double d[3];

    bisection(c, a, e);
    bisection(b, a, c);
    bisection(d, c, e);

    glVertex3f(a[0] * r, a[1] * r, a[2] * r);
    glVertex3f(b[0] * r, b[1] * r, b[2] * r);
    glVertex3f(c[0] * r, c[1] * r, c[2] * r);
    glVertex3f(d[0] * r, d[1] * r, d[2] * r);
}

//-----------------------------------------------------------------------------

void uni::spatch::init(const double *n0, const double *t0, int i0,
                       const double *n1, const double *t1, int i1,
                       const double *n2, const double *t2, int i2,
                       const double *vp, double r, int dd)
{
    double v0[3];
    double v1[3];
    double v2[3];

    v0[0] = n0[0] * r - vp[0];
    v0[1] = n0[1] * r - vp[1];
    v0[2] = n0[2] * r - vp[2];

    v1[0] = n1[0] * r - vp[0];
    v1[1] = n1[1] * r - vp[1];
    v1[2] = n1[2] * r - vp[2];

    v2[0] = n2[0] * r - vp[0];
    v2[1] = n2[1] * r - vp[1];
    v2[2] = n2[2] * r - vp[2];

    v_cp3(n[0], n0);
    v_cp3(n[1], n1);
    v_cp3(n[2], n2);

    v_cp2(t[0], t0);
    v_cp2(t[1], t1);
    v_cp2(t[2], t2);

    i[0][0] = i[0][1] = i0;
    i[1][0] = i[1][1] = i1;
    i[2][0] = i[2][1] = i2;

    d = dd;
    k = fabs(solid_angle(v0, v1, v2));

    N[0] = n0[0] + n1[0] + n2[0];
    N[1] = n0[1] + n1[1] + n2[1];
    N[2] = n0[2] + n1[2] + n2[2];

    normalize(N);

    double d =      DOT3(n0, N);
    d = std::min(d, DOT3(n1, N));
    d = std::min(d, DOT3(n2, N));

    a = acos(d);
}

bool uni::spatch::less(const spatch *S) const
{
    return (k < S->k);
}

bool uni::spatch::more(const spatch *S) const
{
    return (k > S->k);
}

void uni::spatch::link(int ii,
                       int i0,
                       int i1)
{
    if (i[0][0] == ii) i[0][0] = i1;
    if (i[0][1] == ii) i[0][1] = i0;

    if (i[1][0] == ii) i[1][0] = i1;
    if (i[1][1] == ii) i[1][1] = i0;

    if (i[2][0] == ii) i[2][0] = i1;
    if (i[2][1] == ii) i[2][1] = i0;
}

bool uni::spatch::able(const spatch *V) const
{
    if (i[0][0] >= 0 && V[i[0][0]].d < d) return false;
    if (i[0][1] >= 0 && V[i[0][1]].d < d) return false;

    if (i[1][0] >= 0 && V[i[1][0]].d < d) return false;
    if (i[1][1] >= 0 && V[i[1][1]].d < d) return false;

    if (i[2][0] >= 0 && V[i[2][0]].d < d) return false;
    if (i[2][1] >= 0 && V[i[2][1]].d < d) return false;

    return true;
}

bool uni::spatch::test(const double *n0,
                       const double *n1,
                       const double *n2,
                       double r0, double r1, app::frustum_v& frusta) const
{
    // Return true if any part of the given shell falls within any frustum.

    for (app::frustum_i i = frusta.begin(); i != frusta.end(); ++i)
        if ((*i)->test_cap(N, a, r0, r1) >= 0)
            return true;
/*
        if ((*i)->test_shell(n0, n1, n2, r0, r1) >= 0)
            return true;
*/
    return false;
}

bool uni::spatch::subd(spatch *V,
                       int    ii,
                       int&   in,
                       int    im,
                       double r0,
                       double r1,
                       app::frustum_v& F, const double *vp)
{
    int i0 = -1;
    int i1 = -1;
    int i2 = -1;
    int i3 = ii;

    // If this spatch cannot be subdivided, recursively subdivide neighbors.

    while (!able(V))
    {
        if (i[0][0] >= 0 && V[i[0][0]].d < d)
        {
            if (!V[i[0][0]].subd(V, i[0][0], in, im, r0, r1, F, vp))
                return false;
        }

        if (i[1][0] >= 0 && V[i[1][0]].d < d)
        {
            if (!V[i[1][0]].subd(V, i[1][0], in, im, r0, r1, F, vp))
                return false;
        }

        if (i[2][0] >= 0 && V[i[2][0]].d < d)
        {
            if (!V[i[2][0]].subd(V, i[2][0], in, im, r0, r1, F, vp))
                return false;
        }
    }

    // Compute the normal bisection and coordinate midpoint.

    double N[3][3];
    double T[3][2];

    bisection(N[0], n[1], n[2]);
    bisection(N[1], n[2], n[0]);
    bisection(N[2], n[0], n[1]);

    midpoint (T[0], t[1], t[2]);
    midpoint (T[1], t[2], t[0]);
    midpoint (T[2], t[0], t[1]);

    // Check if the new spatches are visible.

    int nn = in;

    if (test(n[0], N[2], N[1], r0, r1, F)) i0 = nn++;
    if (test(N[2], n[1], N[0], r0, r1, F)) i1 = nn++;
    if (test(N[1], N[0], n[2], r0, r1, F)) i2 = nn++;

    if (nn < im)
        in = nn;
    else
        return false;

    // Reset the incoming links.

    if (i[0][0] == i[0][1])
    {
        if (i[0][0] >= 0) V[i[0][0]].link(ii, i0, i1);
    }
    else
    {
        if (i[0][0] >= 0) V[i[0][0]].link(ii, i0, i0);
        if (i[0][1] >= 0) V[i[0][1]].link(ii, i1, i1);
    }

    if (i[1][0] == i[1][1])
    {
        if (i[1][0] >= 0) V[i[1][0]].link(ii, i1, i2);
    }
    else
    {
        if (i[1][0] >= 0) V[i[1][0]].link(ii, i1, i1);
        if (i[1][1] >= 0) V[i[1][1]].link(ii, i2, i2);
    }

    if (i[2][0] == i[2][1])
    {
        if (i[2][0] >= 0) V[i[2][0]].link(ii, i2, i0);
    }
    else
    {
        if (i[2][0] >= 0) V[i[2][0]].link(ii, i2, i2);
        if (i[2][1] >= 0) V[i[2][1]].link(ii, i0, i0);
    }

    // Initialize the new patches.

    if (i0 >= 0) V[i0].init(n[0], t[0], i[0][0],
                            N[2], T[2], i3,
                            N[1], T[1], i[2][1], vp, r0, d + 1);
    if (i1 >= 0) V[i1].init(N[2], T[2], i[0][1],
                            n[1], t[1], i[1][0],
                            N[0], T[0], i3,      vp, r0, d + 1);
    if (i2 >= 0) V[i2].init(N[1], T[1], i3,
                            N[0], T[0], i[1][1],
                            n[2], t[2], i[2][0], vp, r0, d + 1);
    if (i3 >= 0) V[i3].init(N[0], T[0], i2,
                            N[1], T[1], i0,
                            N[2], T[2], i1,      vp, r0, d + 1);

    // HACK

    if (i0 == -1 && i1 == -1 && i2 == -1)
        k = -1;

    return true;
}

void uni::spatch::seed(int line,
                       geonrm& nrm,
                       geopos& pos,
                       geotex& tex,
                       double r, const double *M, const double *I) const
{
    for (int i = 0; i < 3; ++i)
    {
        GLsizei j = line * 3 + i;

        double v[3];
        double V[3];
        double N[3];

        v[0] = n[i][0] * r;
        v[1] = n[i][1] * r;
        v[2] = n[i][2] * r;

        mult_mat_vec3(V, M, v);
        mult_xps_vec3(N, I, n[i]);

        nrm.seed(j, GLfloat(N[0]), GLfloat(N[1]), GLfloat(N[2]), 1.0);
        pos.seed(j, GLfloat(V[0]), GLfloat(V[1]), GLfloat(V[2]), 0.0);
        tex.seed(j, GLfloat(t[i][0]), GLfloat(t[i][1]), 0.0, 0.0);
    }
}

void uni::spatch::draw(const spatch *V, int line,
                       GLsizei cc, GLsizei vc, GLsizei tc) const
{
    // Select the proper index buffer for the given neighbor set.

    GLsizei w = 0;

    if (i[0][0] >= 0 && V[i[0][0]].d < d) w += 4;
    if (i[1][0] >= 0 && V[i[1][0]].d < d) w += 2;
    if (i[2][0] >= 0 && V[i[2][0]].d < d) w += 1;

    // Compute vertex, normal, and index buffer offsets.

    void *vp = (void *) ((line         ) * vc * 4 * sizeof(GLfloat));
    void *np = (void *) ((line + cc    ) * vc * 4 * sizeof(GLfloat));
    void *cp = (void *) ((line + cc * 2) * vc * 4 * sizeof(GLfloat));
    void *tp = (void *) ((w            ) * tc * 3 * sizeof(GLushort));

    // Draw.

    glNormalPointer  (   GL_FLOAT, 4 * sizeof (GLfloat), np);
    glVertexPointer  (3, GL_FLOAT, 4 * sizeof (GLfloat), vp);
    glTexCoordPointer(4, GL_FLOAT, 4 * sizeof (GLfloat), cp);
            
    glDrawElements(GL_TRIANGLES, tc * 3, GL_UNSIGNED_SHORT, tp);
}

void uni::spatch::wire(double r0, double r1) const
{
    double m[3][3];

    bisection(m[0], n[0], n[1]);
    bisection(m[1], n[1], n[2]);
    bisection(m[2], n[2], n[0]);

    glColor4f(1.0f, 1.0f, 0.0f, 0.5f);

    glBegin(GL_LINE_LOOP);
    {
        glVertex3d(n[0][0] * r0, n[0][1] * r0, n[0][2] * r0);
        glVertex3d(n[1][0] * r0, n[1][1] * r0, n[1][2] * r0);
        glVertex3d(n[2][0] * r0, n[2][1] * r0, n[2][2] * r0);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    {
        subdiv(n[0], n[1], r1);
        subdiv(n[1], n[2], r1);
        subdiv(n[2], n[0], r1);
/*
        glVertex3d(n[0][0] * r1, n[0][1] * r1, n[0][2] * r1);
        glVertex3d(n[1][0] * r1, n[1][1] * r1, n[1][2] * r1);
        glVertex3d(n[2][0] * r1, n[2][1] * r1, n[2][2] * r1);
*/
    }
    glEnd();

    glBegin(GL_LINES);
    {
        glVertex3d(n[0][0] * r0, n[0][1] * r0, n[0][2] * r0);
        glVertex3d(n[0][0] * r1, n[0][1] * r1, n[0][2] * r1);
        glVertex3d(n[1][0] * r0, n[1][1] * r0, n[1][2] * r0);
        glVertex3d(n[1][0] * r1, n[1][1] * r1, n[1][2] * r1);
        glVertex3d(n[2][0] * r0, n[2][1] * r0, n[2][2] * r0);
        glVertex3d(n[2][0] * r1, n[2][1] * r1, n[2][2] * r1);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
