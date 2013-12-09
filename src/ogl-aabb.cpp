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

#include <etc-vector.hpp>
#include <ogl-opengl.hpp>
#include <ogl-aabb.hpp>

//-----------------------------------------------------------------------------

ogl::aabb::aabb(const vec3& p, const vec3& q) :
    a(std::min(p[0], q[0]),
      std::min(p[1], q[1]),
      std::min(p[2], q[2])),
    z(std::max(p[0], q[0]),
      std::max(p[1], q[1]),
      std::max(p[2], q[2]))
{
}

ogl::aabb::aabb() :
    a(+std::numeric_limits<double>::max(),
      +std::numeric_limits<double>::max(),
      +std::numeric_limits<double>::max()),
    z(-std::numeric_limits<double>::max(),
      -std::numeric_limits<double>::max(),
      -std::numeric_limits<double>::max())
{
}

ogl::aabb::aabb(const aabb& that, const mat4& M) :
    a(+std::numeric_limits<double>::max(),
      +std::numeric_limits<double>::max(),
      +std::numeric_limits<double>::max()),
    z(-std::numeric_limits<double>::max(),
      -std::numeric_limits<double>::max(),
      -std::numeric_limits<double>::max())
{
    merge(M * vec3(that.a[0], that.a[1], that.a[2]));
    merge(M * vec3(that.z[0], that.a[1], that.a[2]));
    merge(M * vec3(that.a[0], that.z[1], that.a[2]));
    merge(M * vec3(that.z[0], that.z[1], that.a[2]));
    merge(M * vec3(that.a[0], that.a[1], that.z[2]));
    merge(M * vec3(that.z[0], that.a[1], that.z[2]));
    merge(M * vec3(that.a[0], that.z[1], that.z[2]));
    merge(M * vec3(that.z[0], that.z[1], that.z[2]));
}

//-----------------------------------------------------------------------------

void ogl::aabb::merge(const vec3& p)
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

//-----------------------------------------------------------------------------

// Compute the shortest distance from point p to this axis-aligned bounding
// box.  There are 27 possible configurations.  This logic can be optimized
// down to a single cascade of if statements, but at cost to clarity.

double ogl::aabb::get_distance(const vec3& p) const
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

    else if (x1 && y1 && z1) return length(p - vec3(z[0], z[1], z[2]));
    else if (x0 && y1 && z1) return length(p - vec3(a[0], z[1], z[2]));
    else if (x1 && y0 && z1) return length(p - vec3(z[0], a[1], z[2]));
    else if (x0 && y0 && z1) return length(p - vec3(a[0], a[1], z[2]));
    else if (x1 && y1 && z0) return length(p - vec3(z[0], z[1], a[2]));
    else if (x0 && y1 && z0) return length(p - vec3(a[0], z[1], a[2]));
    else if (x1 && y0 && z0) return length(p - vec3(z[0], a[1], a[2]));
    else if (x0 && y0 && z0) return length(p - vec3(a[0], a[1], a[2]));

    // Edge cases are next.

    else if (!z0 && !z1)
    {
        if      (x0 && y0) return length(p - vec3(a[0], a[1], p[2]));
        else if (x1 && y0) return length(p - vec3(z[0], a[1], p[2]));
        else if (x0 && y1) return length(p - vec3(a[0], z[1], p[2]));
        else if (x1 && y1) return length(p - vec3(z[0], z[1], p[2]));
    }
    else if (!y0 && !y1)
    {
        if      (x0 && z0) return length(p - vec3(a[0], p[1], a[2]));
        else if (x1 && z0) return length(p - vec3(z[0], p[1], a[2]));
        else if (x0 && z1) return length(p - vec3(a[0], p[1], z[2]));
        else if (x1 && z1) return length(p - vec3(z[0], p[1], z[2]));
    }
    else if (!x0 && !x1)
    {
        if      (y0 && z0) return length(p - vec3(p[0], a[1], a[2]));
        else if (y1 && z0) return length(p - vec3(p[0], z[1], a[2]));
        else if (y0 && z1) return length(p - vec3(p[0], a[1], z[2]));
        else if (y1 && y1) return length(p - vec3(p[0], z[1], z[2]));
    }

    // Side cases are least likely.

    if      (z1) return p[2] - z[2];
    else if (z0) return a[2] - p[2];
    else if (y1) return p[1] - z[1];
    else if (y0) return a[1] - p[1];
    else if (x1) return p[0] - z[0];
    else         return a[0] - p[0];
}

ogl::range ogl::aabb::get_range(const vec4& P) const
{
    return range(min(P), max(P));
}

ogl::range ogl::aabb::get_range(const vec4& P, const mat4& M) const
{
    const mat4 T = transpose(M);

    return range(min(T * P), max(T * P));
}

//-----------------------------------------------------------------------------

bool ogl::aabb::test(const vec4 *V, int n) const
{
    for (int i = 0; i < n; ++i)
        if (max(V[i]) < 0)
            return false;

    return true;
}

bool ogl::aabb::test(const vec4 *V, int  n,
                     const mat4& M, int &hint) const
{
    const mat4 T = transpose(M);

    // Use the hint to check the likely cull plane.

    if (max(T * V[hint]) < 0) return false;

    // The hint was no good.  Check all the other planes.

    for (int i = 0; i < n; ++i)
        if (i != hint && max(T * V[i]) < 0)
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

double ogl::aabb::max(const vec4& P) const
{
    // Find the corner most positive w.r.t the plane.  Return the distance.

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return vec4(z[0], z[1], z[2], 1) * P;
            else
                return vec4(z[0], z[1], a[2], 1) * P;
        else
            if (P[2] > 0)
                return vec4(z[0], a[1], z[2], 1) * P;
            else
                return vec4(z[0], a[1], a[2], 1) * P;
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return vec4(a[0], z[1], z[2], 1) * P;
            else
                return vec4(a[0], z[1], a[2], 1) * P;
        else
            if (P[2] > 0)
                return vec4(a[0], a[1], z[2], 1) * P;
            else
                return vec4(a[0], a[1], a[2], 1) * P;
}

double ogl::aabb::min(const vec4& P) const
{
    // Find the corner most negative w.r.t the plane.  Return the distance.

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return vec4(a[0], a[1], a[2], 1) * P;
            else
                return vec4(a[0], a[1], z[2], 1) * P;
        else
            if (P[2] > 0)
                return vec4(a[0], z[1], a[2], 1) * P;
            else
                return vec4(a[0], z[1], z[2], 1) * P;
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return vec4(z[0], a[1], a[2], 1) * P;
            else
                return vec4(z[0], a[1], z[2], 1) * P;
        else
            if (P[2] > 0)
                return vec4(z[0], z[1], a[2], 1) * P;
            else
                return vec4(z[0], z[1], z[2], 1) * P;
}

//-----------------------------------------------------------------------------
