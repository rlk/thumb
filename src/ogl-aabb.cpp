//  Copyright (C) 2007-2011 Robert Kooima
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

#include <algorithm>

#include <etc-math.hpp>
#include <ogl-opengl.hpp>
#include <ogl-aabb.hpp>

//-----------------------------------------------------------------------------

double ogl::aabb::max(const double *P) const
{
    // Find the corner most positive w.r.t the plane.  Return the distance.

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return z[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3];
            else
                return z[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3];
        else
            if (P[2] > 0)
                return z[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3];
            else
                return z[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3];
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return a[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3];
            else
                return a[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3];
        else
            if (P[2] > 0)
                return a[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3];
            else
                return a[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3];
}

double ogl::aabb::min(const double *P) const
{
    // Find the corner most negative w.r.t the plane.  Return the distance.

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return a[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3];
            else
                return a[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3];
        else
            if (P[2] > 0)
                return a[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3];
            else
                return a[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3];
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return z[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3];
            else
                return z[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3];
        else
            if (P[2] > 0)
                return z[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3];
            else
                return z[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3];
}

double ogl::aabb::max(const double *M, const double *V) const
{
    double P[4];

    // Transform the clipping plane into the bounding box's local space.

    mult_xps_vec4(P, M, V);

    // Find the corner most positive w.r.t the plane.  Return distance.

    return max(P);
}

//-----------------------------------------------------------------------------

ogl::aabb::aabb(double x0, double y0, double z0, 
                double x1, double y1, double z1)
{
    a[0] = x0;
    a[1] = y0;
    a[2] = z0;

    z[0] = x1;
    z[1] = y1;
    z[2] = z1;
}

ogl::aabb::aabb()
{
    a[0] = +std::numeric_limits<double>::max();
    a[1] = +std::numeric_limits<double>::max();
    a[2] = +std::numeric_limits<double>::max();

    z[0] = -std::numeric_limits<double>::max();
    z[1] = -std::numeric_limits<double>::max();
    z[2] = -std::numeric_limits<double>::max();
}

//-----------------------------------------------------------------------------

void ogl::aabb::merge(double px, double py, double pz)
{
    a[0] = std::min(a[0], px);
    a[1] = std::min(a[1], py);
    a[2] = std::min(a[2], pz);

    z[0] = std::max(z[0], px);
    z[1] = std::max(z[1], py);
    z[2] = std::max(z[2], pz);
}

void ogl::aabb::merge(const aabb& that)
{
    a[0] = std::min(a[0], that.a[0]);
    a[1] = std::min(a[1], that.a[1]);
    a[2] = std::min(a[2], that.a[2]);

    z[0] = std::max(z[0], that.z[0]);
    z[1] = std::max(z[1], that.z[1]);
    z[2] = std::max(z[2], that.z[2]);
}

//-----------------------------------------------------------------------------

#define DIST2(x0, y0, x1, y1)         sqrt((x0 - x1) * (x0 - x1) + \
                                           (y0 - y1) * (y0 - y1))
#define DIST3(x0, y0, z0, x1, y1, z1) sqrt((x0 - x1) * (x0 - x1) + \
                                           (y0 - y1) * (y0 - y1) + \
                                           (z0 - z1) * (z0 - z1))

// Compute the shortest distance from point p to this axis-aligned bounding
// box.  There are 27 possible configurations.  This logic can be optimized
// down to a single cascade of if statements, but at cost to clarity.
/*
double ogl::aabb::get_distance(const double *p) const
{
    const bool x1 = (p[0] > z[0]);
    const bool y1 = (p[1] > z[1]);
    const bool z1 = (p[2] > z[2]);
    const bool x0 = (p[0] < a[0]);
    const bool y0 = (p[1] < a[1]);
    const bool z0 = (p[2] < a[2]);

    // Eliminate the degenerate case to simplify the subsequent logic.

    if (!x0 && !y0 && !z0 && !x1 && !y1 && !z1) return 0.0;

    // Corner cases are most common.

    else if (x1 && y1 && z1) return DIST3(p[0], p[1], p[2], z[0], z[1], z[2]);
    else if (x0 && y1 && z1) return DIST3(p[0], p[1], p[2], a[0], z[1], z[2]);
    else if (x1 && y0 && z1) return DIST3(p[0], p[1], p[2], z[0], a[1], z[2]);
    else if (x0 && y0 && z1) return DIST3(p[0], p[1], p[2], a[0], a[1], z[2]);
    else if (x1 && y1 && z0) return DIST3(p[0], p[1], p[2], z[0], z[1], a[2]);
    else if (x0 && y1 && z0) return DIST3(p[0], p[1], p[2], a[0], z[1], a[2]);
    else if (x1 && y0 && z0) return DIST3(p[0], p[1], p[2], z[0], a[1], a[2]);
    else if (x0 && y0 && z0) return DIST3(p[0], p[1], p[2], a[0], a[1], a[2]);

    // Edge cases are next.

    else if (!z0 && !z1)
    {
        if      (x0 && y0) return DIST2(p[0], p[1], a[0], a[1]);
        else if (x1 && y0) return DIST2(p[0], p[1], z[0], a[1]);
        else if (x0 && y1) return DIST2(p[0], p[1], a[0], z[1]);
        else               return DIST2(p[0], p[1], z[0], z[1]);
    }
    else if (!y0 && !y1)
    {
        if      (x0 && z0) return DIST2(p[0], p[2], a[0], a[2]);
        else if (x1 && z0) return DIST2(p[0], p[2], z[0], a[2]);
        else if (x0 && z1) return DIST2(p[0], p[2], a[0], z[2]);
        else               return DIST2(p[0], p[2], z[0], z[2]);
    }
    else if (!x0 && !x1)
    {
        if      (y0 && z0) return DIST2(p[1], p[2], a[1], a[2]);
        else if (y1 && z0) return DIST2(p[1], p[2], z[1], a[2]);
        else if (y0 && z1) return DIST2(p[1], p[2], a[1], z[2]);
        else               return DIST2(p[1], p[2], z[1], z[2]);
    }

    // Side cases are least likely.

    else if (z1) return p[2] - z[2];
    else if (z0) return a[2] - p[2];
    else if (y1) return p[1] - z[1];
    else if (y0) return a[1] - p[1];
    else if (x1) return p[0] - z[0];
    else         return a[0] - p[0];
}
*/
double ogl::aabb::get_distance(const double *p) const
{
    const bool x1 = (p[0] > z[0]);
    const bool y1 = (p[1] > z[1]);
    const bool z1 = (p[2] > z[2]);
    const bool x0 = (p[0] < a[0]);
    const bool y0 = (p[1] < a[1]);
    const bool z0 = (p[2] < a[2]);

    // Eliminate the degenerate case to simplify the subsequent logic.

    if (!x0 && !y0 && !z0 && !x1 && !y1 && !z1) return 0.0;

    // Corner cases are most common.

    else if (x1 && y1 && z1) return DIST3(p[0], p[1], p[2], z[0], z[1], z[2]);
    else if (x0 && y1 && z1) return DIST3(p[0], p[1], p[2], a[0], z[1], z[2]);
    else if (x1 && y0 && z1) return DIST3(p[0], p[1], p[2], z[0], a[1], z[2]);
    else if (x0 && y0 && z1) return DIST3(p[0], p[1], p[2], a[0], a[1], z[2]);
    else if (x1 && y1 && z0) return DIST3(p[0], p[1], p[2], z[0], z[1], a[2]);
    else if (x0 && y1 && z0) return DIST3(p[0], p[1], p[2], a[0], z[1], a[2]);
    else if (x1 && y0 && z0) return DIST3(p[0], p[1], p[2], z[0], a[1], a[2]);
    else if (x0 && y0 && z0) return DIST3(p[0], p[1], p[2], a[0], a[1], a[2]);

    // Edge cases are next.

    else if (!z0 && !z1)
    {
        if      (x0 && y0) return DIST2(p[0], p[1], a[0], a[1]);
        else if (x1 && y0) return DIST2(p[0], p[1], z[0], a[1]);
        else if (x0 && y1) return DIST2(p[0], p[1], a[0], z[1]);
        else if (x1 && y1) return DIST2(p[0], p[1], z[0], z[1]);
    }
    else if (!y0 && !y1)
    {
        if      (x0 && z0) return DIST2(p[0], p[2], a[0], a[2]);
        else if (x1 && z0) return DIST2(p[0], p[2], z[0], a[2]);
        else if (x0 && z1) return DIST2(p[0], p[2], a[0], z[2]);
        else if (x1 && z1) return DIST2(p[0], p[2], z[0], z[2]);
    }
    else if (!x0 && !x1)
    {
        if      (y0 && z0) return DIST2(p[1], p[2], a[1], a[2]);
        else if (y1 && z0) return DIST2(p[1], p[2], z[1], a[2]);
        else if (y0 && z1) return DIST2(p[1], p[2], a[1], z[2]);
        else if (y1 && y1) return DIST2(p[1], p[2], z[1], z[2]);
    }

    // Side cases are least likely.

    if      (z1) return p[2] - z[2];
    else if (z0) return a[2] - p[2];
    else if (y1) return p[1] - z[1];
    else if (y0) return a[1] - p[1];
    else if (x1) return p[0] - z[0];
    else         return a[0] - p[0];
}

ogl::range ogl::aabb::get_range(const double *P) const
{
    return range(min(P), max(P));
}

ogl::range ogl::aabb::get_range(const double *M, const double *V) const
{
    double P[4];

    // Transform the clipping plane into the bounding box's local space.

    mult_xps_vec4(P, M, V);

    return range(min(P), max(P));
}

//-----------------------------------------------------------------------------

bool ogl::aabb::test(const double *V, int n) const
{
    for (int i = 0; i < n; ++i)
        if (max(V + 4 * i) < 0)
            return false;

    return true;
}

bool ogl::aabb::test(const double *M, int  n,
                     const double *V, int &hint) const
{
    // Use the hint to check the likely cull plane.

    if (max(M, V + 4 * hint) < 0) return false;

    // The hint was no good.  Check all the other planes.

    for (int i = 0; i < n; ++i)
        if (i != hint && max(M, V + 4 * i) < 0)
        {
            // Plane i is a hit.  Set it as the hint for next time.

            hint = i;

            // Return invisible.

            return false;
        }

    // Nothing clipped.  Return visible.

    return true;
}

//-----------------------------------------------------------------------------

void ogl::aabb::draw(bool n, bool s, bool e, bool w) const
{
//  double d = (z[0] - a[0]) / 20.0;

    double d = 0.1;

    glBegin(GL_LINES);
    {
        if (n) glColor3f(1.0f, 1.0f, 0.0f); else glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3d(a[0], 0.0, a[2] + d);
        glVertex3d(z[0], 0.0, a[2] + d);

        if (s) glColor3f(1.0f, 1.0f, 0.0f); else glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3d(a[0], 0.0, z[2] - d);
        glVertex3d(z[0], 0.0, z[2] - d);

        if (e) glColor3f(1.0f, 1.0f, 0.0f); else glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3d(z[0] - d, 0.0, a[2]);
        glVertex3d(z[0] - d, 0.0, z[2]);

        if (w) glColor3f(1.0f, 1.0f, 0.0f); else glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3d(a[0] + d, 0.0, a[2]);
        glVertex3d(a[0] + d, 0.0, z[2]);
    }
    glEnd();
}

void ogl::aabb::draw() const
{
    glBegin(GL_LINES);
    {
        glVertex3d(a[0], a[1], a[2]);
        glVertex3d(z[0], a[1], a[2]);
        glVertex3d(a[0], z[1], a[2]);
        glVertex3d(z[0], z[1], a[2]);
        glVertex3d(a[0], a[1], z[2]);
        glVertex3d(z[0], a[1], z[2]);
        glVertex3d(a[0], z[1], z[2]);
        glVertex3d(z[0], z[1], z[2]);

        glVertex3d(a[0], a[1], a[2]);
        glVertex3d(a[0], z[1], a[2]);
        glVertex3d(z[0], a[1], a[2]);
        glVertex3d(z[0], z[1], a[2]);
        glVertex3d(a[0], a[1], z[2]);
        glVertex3d(a[0], z[1], z[2]);
        glVertex3d(z[0], a[1], z[2]);
        glVertex3d(z[0], z[1], z[2]);

        glVertex3d(a[0], a[1], a[2]);
        glVertex3d(a[0], a[1], z[2]);
        glVertex3d(z[0], a[1], a[2]);
        glVertex3d(z[0], a[1], z[2]);
        glVertex3d(a[0], z[1], a[2]);
        glVertex3d(a[0], z[1], z[2]);
        glVertex3d(z[0], z[1], a[2]);
        glVertex3d(z[0], z[1], z[2]);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
