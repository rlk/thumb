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

#include <algorithm>

#include "matrix.hpp"
#include "ogl-opengl.hpp"
#include "ogl-aabb.hpp"

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

ogl::aabb::aabb()
{
    a[0] = +std::numeric_limits<double>::max();
    a[1] = +std::numeric_limits<double>::max();
    a[2] = +std::numeric_limits<double>::max();

    z[0] = -std::numeric_limits<double>::max();
    z[1] = -std::numeric_limits<double>::max();
    z[2] = -std::numeric_limits<double>::max();
}

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

ogl::range ogl::aabb::get_range(const double *M, const double *V) const
{
    double P[4];

    // Transform the clipping plane into the bounding box's local space.

    mult_xps_vec4(P, M, V);

    return range(min(P), max(P));
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
