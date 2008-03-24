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

#include <iostream>
#include <iomanip>
#include <cmath>

#include "matrix.hpp"
#include "patch.hpp"

//-----------------------------------------------------------------------------

uni::point::point(const double *N)
    : count(0), frame(0)
{
    n[0] = N[0];
    n[1] = N[1];
    n[2] = N[2];

    normalize(n);
}

uni::point::point(const point *a, const point *b) : count(0), frame(0)
{
    bisection(n, a->n, b->n);
}

uni::point *uni::point::ref()
{
    count++;
    return this;
}

uni::point *uni::point::unref()
{
    if (--count)
        return this;
    else
    {
        delete this;
        return 0;
    }
}

void uni::point::transform(const double *M,
                           const double *I, double r, int f)
{
    if (frame < f)
    {
        frame = f;

        double v[3];

        v[0] = n[0] * r;
        v[1] = n[1] * r;
        v[2] = n[2] * r;

        mult_mat_vec3(V, M, v);
        mult_xps_vec3(N, I, n);

        normalize(N);
    }
}

void uni::point::seed(geonrm& nrm, geopos& pos, geotex& tex,
                      GLsizei i, double u, double v)
{
    nrm.seed(i, GLfloat(N[0]), GLfloat(N[1]), GLfloat(N[2]), 1.0);
    pos.seed(i, GLfloat(V[0]), GLfloat(V[1]), GLfloat(V[2]), 0.0);
    tex.seed(i, GLfloat(u   ), GLfloat(v   ), GLfloat(n[0]), GLfloat(n[2]));
}

void uni::point::draw(double r) const
{
    glVertex3d(V[0] + N[0] * r,
               V[1] + N[1] * r,
               V[2] + N[2] * r);
}

//-----------------------------------------------------------------------------

uni::patch::patch(point *P0, const double *t0,
                  point *P1, const double *t1,
                  point *P2, const double *t2, double rr, double r0, double r1)
    : rr(rr), r0(r0), r1(r1)
{
    C[0] =  0;
    C[1] =  0;
    C[2] =  0;
    C[3] =  0;

    P[0] = P0->ref();
    P[1] = P1->ref();
    P[2] = P2->ref();

    t[0] = t0[0];
    t[1] = t0[1];
    t[2] = t1[0];
    t[3] = t1[1];
    t[4] = t2[0];
    t[5] = t2[1];

    // Compute the normal of this patch.

    const double *n0 = P[0]->get();
    const double *n1 = P[1]->get();
    const double *n2 = P[2]->get();

    n[0] = n0[0] + n1[0] + n2[0];
    n[1] = n0[1] + n1[1] + n2[1];
    n[2] = n0[2] + n1[2] + n2[2];

    normalize(n);

    r2 = r1 / DOT3(n, n0);
    a  = acos(DOT3(n, n0));

    // Compute the area.

    double e[3][3];

    e[0][0] = (n1[0] - n0[0]) * r0;
    e[0][1] = (n1[1] - n0[1]) * r0;
    e[0][2] = (n1[2] - n0[2]) * r0;

    e[1][0] = (n2[0] - n0[0]) * r0;
    e[1][1] = (n2[1] - n0[1]) * r0;
    e[1][2] = (n2[2] - n0[2]) * r0;

    crossprod(e[2], e[0], e[1]);

    area = 0.5 * sqrt(DOT3(e[2], e[2]));
}

uni::patch::~patch()
{
    // Dereference the points.

    P[0]->unref();
    P[1]->unref();
    P[2]->unref();

    // Delete the child patches.

    if (C[0]) delete C[0];
    if (C[1]) delete C[1];
    if (C[2]) delete C[2];
    if (C[3]) delete C[3];
}

//-----------------------------------------------------------------------------

bool uni::patch::leaf()
{
    return (C[0] == 0 && C[1] == 0 && C[2] == 0 && C[3] == 0);
}

double get_radius(const double *c, const double *n, double r)
{
    double d[3];

    d[0] = n[0] * r - c[0];
    d[1] = n[1] * r - c[1];
    d[2] = n[2] * r - c[2];

    return sqrt(DOT3(d, d));
}

int uni::patch::visible(app::frustum_v& frusta)
{
    int d;

    const double *n0 = P[0]->get();
    const double *n1 = P[1]->get();
    const double *n2 = P[2]->get();

    for (app::frustum_i i = frusta.begin(); i != frusta.end(); ++i)
        if ((d = (*i)->test_shell(n0, n1, n2, r0, r1)) >= 0)
            return 0;

    return -1;
}

double uni::patch::value(const double *p)
{
    // Compute the LOD value of this patch.

    const double *n0 = P[0]->get();
    const double *n1 = P[1]->get();
    const double *n2 = P[2]->get();

    double d[3], k = (r0 + r1) / 6.0;

    d[0] = (n0[0] + n1[0] + n2[0]) * k - p[0];
    d[1] = (n0[1] + n1[1] + n2[1]) * k - p[1];
    d[2] = (n0[2] + n1[2] + n2[2]) * k - p[2];
    
    return area / DOT3(d, d);
}


void uni::patch::wire(GLfloat r, GLfloat g, GLfloat b)
{
    if (leaf())
    {
        double dr = 0.5 * (r1 + r0) - rr;

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        {
            P[0]->draw(dr);
            P[1]->draw(dr);
            P[2]->draw(dr);
        }
        glEnd();

        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

        glBegin(GL_LINE_LOOP);
        {
            P[0]->draw(dr);
            P[1]->draw(dr);
            P[2]->draw(dr);
        }
        glEnd();
    }
    else
    {
        if (C[0]) C[0]->wire(r, g, b);
        if (C[1]) C[1]->wire(r, g, b);
        if (C[2]) C[2]->wire(r, g, b);
        if (C[3]) C[3]->wire(r, g, b);
    }
}

//-----------------------------------------------------------------------------

void uni::patch::seed(geonrm& nrm, geopos& pos, geotex& tex,
                      const double *M,
                      const double *I, int frame)
{
    if (leaf())
    {
        GLsizei i0 = cache * 3 + 0;
        GLsizei i1 = cache * 3 + 1;
        GLsizei i2 = cache * 3 + 2;

        // Update the vertex, normal, and texture coordinate caches.

        P[0]->transform(M, I, rr, frame);
        P[1]->transform(M, I, rr, frame);
        P[2]->transform(M, I, rr, frame);

        P[0]->seed(nrm, pos, tex, i0, t[0], t[1]);
        P[1]->seed(nrm, pos, tex, i1, t[2], t[3]);
        P[2]->seed(nrm, pos, tex, i2, t[4], t[5]);
/*
        tex.seed(i0, GLfloat((t[0])),
                     GLfloat((t[1])),
                     GLfloat((t[0] + PI) * 0.5  / PI),
                     GLfloat((t[1] + PI  * 0.5) / PI));
        tex.seed(i1, GLfloat((t[2])),
                     GLfloat((t[3])),
                     GLfloat((t[2] + PI) * 0.5  / PI),
                     GLfloat((t[3] + PI  * 0.5) / PI));
        tex.seed(i2, GLfloat((t[4])),
                     GLfloat((t[5])),
                     GLfloat((t[4] + PI) * 0.5  / PI),
                     GLfloat((t[5] + PI  * 0.5) / PI));
*/
    }
    else
    {
        if (C[0]) C[0]->seed(nrm, pos, tex, M, I, frame);
        if (C[1]) C[1]->seed(nrm, pos, tex, M, I, frame);
        if (C[2]) C[2]->seed(nrm, pos, tex, M, I, frame);
        if (C[3]) C[3]->seed(nrm, pos, tex, M, I, frame);
    }
}

void uni::patch::view(GLsizei c, const GLfloat *p)
{
    if (leaf())
    {
        if (cache < c)
        {
            double dr0 = double(p[cache * 4 + 2]);
            double dr1 = double(p[cache * 4 + 3]);

            r0 = rr + dr0;
            r1 = rr + dr1;
        }
    }
    else
    {
        if (C[0]) C[0]->view(c, p);
        if (C[1]) C[1]->view(c, p);
        if (C[2]) C[2]->view(c, p);
        if (C[3]) C[3]->view(c, p);
    }
}

uni::patch *uni::patch::step(context& ctx,
                             app::frustum_v& frusta,
                             const double *p,
                             double bias, int d, GLsizei& count)
{
    // Extend or prune the patch tree.

    if ((d > 0) || (d = visible(frusta)) >= 0)
    {
        // This patch is still visible.

        if (value(p) * bias > 1.0)  // HACK
        {
            // This patch is too large.  Subdivide it.

            point *Q[3];

            Q[0] = ctx.get_point(0);
            Q[1] = ctx.get_point(1);
            Q[2] = ctx.get_point(2);

            double s[6];

            midpoint(s + 0, t + 0, t + 2);
            midpoint(s + 2, t + 2, t + 4);
            midpoint(s + 4, t + 4, t + 0);

            // Try to create any nonexistant children.

            if (C[0] == 0)
            {
                if (!Q[0]) { Q[0] = new point(P[0], P[1]); }
                if (!Q[2]) { Q[2] = new point(P[2], P[0]); }

                C[0] = new patch(P[0], t+0, Q[0], s+0, Q[2], s+4, rr, r0, r1);
            }

            if (C[1] == 0)
            {
                if (!Q[0]) { Q[0] = new point(P[0], P[1]); }
                if (!Q[1]) { Q[1] = new point(P[1], P[2]); }

                C[1] = new patch(Q[0], s+0, P[1], t+2, Q[1], s+2, rr, r0, r1);
            }

            if (C[2] == 0)
            {
                if (!Q[2]) { Q[2] = new point(P[2], P[0]); }
                if (!Q[1]) { Q[1] = new point(P[1], P[2]); }

                C[2] = new patch(Q[2], s+4, Q[1], s+2, P[2], t+4, rr, r0, r1);
            }

            if (C[3] == 0)
            {
                if (!Q[0]) { Q[0] = new point(P[0], P[1]); }
                if (!Q[1]) { Q[1] = new point(P[1], P[2]); }
                if (!Q[2]) { Q[2] = new point(P[2], P[0]); }

                C[3] = new patch(Q[0], s+0, Q[1], s+2, Q[2], s+4, rr, r0, r1);
            }

            // Recursively refine any children.

            if (C[0])
            {
                context c(ctx, 0);
                C[0] = C[0]->step(c, frusta, p, bias, d, count);
            }
            if (C[1])
            {
                context c(ctx, 1);
                C[1] = C[1]->step(c, frusta, p, bias, d, count);
            }
            if (C[2])
            {
                context c(ctx, 2);
                C[2] = C[2]->step(c, frusta, p, bias, d, count);
            }
            if (C[3])
            {
                context c(ctx, 3);
                C[3] = C[3]->step(c, frusta, p, bias, d, count);
            }

            // If this patch passed the visibility test and is large enough to
            // subdivide, yet has no children at this point, then it must have
            // passed erroneously due to a coarse bounding volume.  Delete it.

            if (leaf())
            {
                delete this;
                return 0;
            }
        }
        else
        {
            // This patch is too small.  Delete any children.

            if (C[3]) delete C[3];
            if (C[2]) delete C[2];
            if (C[1]) delete C[1];
            if (C[0]) delete C[0];

            C[0] = C[1] = C[2] = C[3] = 0;
        }

        // Assign each leaf patch a position in the geometry cache.

        if (leaf()) cache = count++;

        return this;
    }
    else
    {
        // This patch is no longer visible.

        delete this;
        return 0;
    }
}

/*
void uni::patch::prep(GLsizei w)
{
    if (leaf())
    {
        glRecti(GLint(0), GLint(cache), GLint(w), GLint(cache) + 1);
    }
    else
    {
        if (C[0]) C[0]->prep(w);
        if (C[1]) C[1]->prep(w);
        if (C[2]) C[2]->prep(w);
        if (C[3]) C[3]->prep(w);
    }
}
*/
void uni::patch::draw(context& ctx, GLsizei cc, GLsizei vc, GLsizei tc)
{
    if (leaf())
    {
        // This patch has no children.  Draw it.

        if (cache < cc)
        {
            // Select the proper index buffer for the given neighbor set.

            GLsizei w = ((ctx.get_local(0) ? 0 : 4) +
                         (ctx.get_local(1) ? 0 : 2) +
                         (ctx.get_local(2) ? 0 : 1));

            // Compute vertex, normal, and index buffer offsets.

            void *vp = (void *) ((cache         ) * vc * 4 * sizeof(GLfloat));
            void *np = (void *) ((cache + cc    ) * vc * 4 * sizeof(GLfloat));
            void *cp = (void *) ((cache + cc * 2) * vc * 4 * sizeof(GLfloat));
            void *tp = (void *) ((w             ) * tc * 3 * sizeof(GLushort));

            // Draw.

            glNormalPointer  (   GL_FLOAT, 4 * sizeof (GLfloat), np);
            glVertexPointer  (3, GL_FLOAT, 4 * sizeof (GLfloat), vp);
            glTexCoordPointer(4, GL_FLOAT, 4 * sizeof (GLfloat), cp);
            
            glDrawElements(GL_TRIANGLES, tc * 3, GL_UNSIGNED_SHORT, tp);
        }
    }
    else
    {
        // This patch has children.  Find their neighbors and draw.

        if (C[0]) { context c(ctx, 0); C[0]->draw(c, cc, vc, tc); }
        if (C[1]) { context c(ctx, 1); C[1]->draw(c, cc, vc, tc); }
        if (C[2]) { context c(ctx, 2); C[2]->draw(c, cc, vc, tc); }
        if (C[3]) { context c(ctx, 3); C[3]->draw(c, cc, vc, tc); }
    }
}
/*
void uni::patch::wire()
{
    if (leaf())
    {
    }
    else
    {
        // This patch has children.  Find their neighbors and wire.

        if (C[0]) C[0]->wire();
        if (C[1]) C[1]->wire();
        if (C[2]) C[2]->wire();
        if (C[3]) C[3]->wire();
    }
}
*/
//-----------------------------------------------------------------------------

uni::patch *uni::context::get_loc_L(int i)
{
    return N[i] ? N[i]->get_child((n[i]    )    ) : 0;
}

uni::patch *uni::context::get_loc_R(int i)
{
    return N[i] ? N[i]->get_child((n[i] + 1) % 3) : 0;
}

uni::context::context(patch *P, patch *N0, int n0, 
                                patch *N1, int n1, 
                      patch *N2, int n2) : P(P), d(0)
{
    N[0] = N0; n[0] = n0;
    N[1] = N1; n[1] = n1;
    N[2] = N2; n[2] = n2;
}

uni::context::context(context& con, int i)
{
    // Step down to child i of P.

    P = con.get_child(i);
    d = con.get_depth() + 1;

    switch (i)
    {
    case 0:
        N[0] = con.get_loc_R(0); n[0] = con.n[0];
        N[1] = con.get_child(3); n[1] = 2;
        N[2] = con.get_loc_L(2); n[2] = con.n[2];
        break;
    case 1:
        N[0] = con.get_loc_L(0); n[0] = con.n[0];
        N[1] = con.get_loc_R(1); n[1] = con.n[1];
        N[2] = con.get_child(3); n[2] = 0;
        break;
    case 2:
        N[0] = con.get_child(3); n[0] = 1;
        N[1] = con.get_loc_L(1); n[1] = con.n[1];
        N[2] = con.get_loc_R(2); n[2] = con.n[2];
        break;
    case 3:
        N[0] = con.get_child(1); n[0] = 2;
        N[1] = con.get_child(2); n[1] = 0;
        N[2] = con.get_child(0); n[2] = 1;
        break;
    }
}

uni::patch *uni::context::get_patch()
{
    return P;
}

uni::patch *uni::context::get_child(int i)
{
    return P->get_child(i);
}

uni::patch *uni::context::get_local(int i)
{
    return N[i];
}

uni::point *uni::context::get_point(int i)
{
    patch *C;

    if (N[i])
    {
        if ((C = N[i]->get_child(n[i])))
            return C->get_point((n[i] + 1) % 3);
    }

    return 0;
}

//-----------------------------------------------------------------------------
