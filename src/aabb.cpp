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

#include "aabb.hpp"
#include "matrix.hpp"

//-----------------------------------------------------------------------------

ogl::aabb::aabb()
{
    a[0] = +std::numeric_limits<GLfloat>::max();
    a[1] = +std::numeric_limits<GLfloat>::max();
    a[2] = +std::numeric_limits<GLfloat>::max();

    z[0] = -std::numeric_limits<GLfloat>::max();
    z[1] = -std::numeric_limits<GLfloat>::max();
    z[2] = -std::numeric_limits<GLfloat>::max();
}

void ogl::aabb::merge(const GLfloat *p)
{
    a[0] = std::min(a[0], p[0]);
    a[1] = std::min(a[1], p[1]);
    a[2] = std::min(a[2], p[2]);

    z[0] = std::max(z[0], p[0]);
    z[1] = std::max(z[1], p[1]);
    z[2] = std::max(z[2], p[2]);
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

GLfloat ogl::aabb::dist(const GLfloat *M, const GLfloat *V)
{
    GLfloat P[4];

    // Transform the clipping plane into the bounding box's local space.

    mult_xps_vec(P, M, V);

    // Find the corner most positive w.r.t the plane.  Return distance.

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

bool ogl::aabb::test(const GLfloat *M, int  n,
                     const GLfloat *V, int &hint)
{
    // Use the hint to check the likely cull plane.

    if (dist(M, V + 4 * hint) < 0) return false;

    // The hint was no good.  Check all the other planes.

    for (int i = 0; i < n; ++i)
        if (i != hint && dist(M, V + 4 * i) < 0)
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
