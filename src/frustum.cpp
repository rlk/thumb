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

#include <cmath>
#include <cstring>

#include "frustum.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

void app::frustum::get_calibration(double *M)
{
    // Extract the calibration matrix from the serialization node.

    app::node curr;

    if (node && (curr = find(node, "calibration")))
    {
        M[ 0] = get_attr_f(curr, "m0", 1.0);
        M[ 1] = get_attr_f(curr, "m1", 0.0);
        M[ 2] = get_attr_f(curr, "m2", 0.0);
        M[ 3] = get_attr_f(curr, "m3", 0.0);
        M[ 4] = get_attr_f(curr, "m4", 0.0);
        M[ 5] = get_attr_f(curr, "m5", 1.0);
        M[ 6] = get_attr_f(curr, "m6", 0.0);
        M[ 7] = get_attr_f(curr, "m7", 0.0);
        M[ 8] = get_attr_f(curr, "m8", 0.0);
        M[ 9] = get_attr_f(curr, "m9", 0.0);
        M[10] = get_attr_f(curr, "mA", 1.0);
        M[11] = get_attr_f(curr, "mB", 0.0);
        M[12] = get_attr_f(curr, "mC", 0.0);
        M[13] = get_attr_f(curr, "mD", 0.0);
        M[14] = get_attr_f(curr, "mE", 0.0);
        M[15] = get_attr_f(curr, "mF", 1.0);
    }
    else load_idt(M);
}

void app::frustum::set_calibration(const double *M)
{
    // Update the calibration matrix in the serialization node.

    app::node curr;

    if (node && (curr = find(node, "calibration")))
    {
        set_attr_f(curr, "m0", M[ 0]);
        set_attr_f(curr, "m1", M[ 1]);
        set_attr_f(curr, "m2", M[ 2]);
        set_attr_f(curr, "m3", M[ 3]);
        set_attr_f(curr, "m4", M[ 4]);
        set_attr_f(curr, "m5", M[ 5]);
        set_attr_f(curr, "m6", M[ 6]);
        set_attr_f(curr, "m7", M[ 7]);
        set_attr_f(curr, "m8", M[ 8]);
        set_attr_f(curr, "m9", M[ 9]);
        set_attr_f(curr, "mA", M[10]);
        set_attr_f(curr, "mB", M[11]);
        set_attr_f(curr, "mC", M[12]);
        set_attr_f(curr, "mD", M[13]);
        set_attr_f(curr, "mE", M[14]);
        set_attr_f(curr, "mF", M[15]);
    }
}

void app::frustum::calc_corner_4(double *c0,
                                 double *c1,
                                 double *c2,
                                 double *c3, double aspect, double fov)
{
    // Compute screen corners given perspective field-of-view and aspect ratio.

    const double x = tan(RAD(fov * 0.5));
    const double y = x / aspect;

    c0[0] = -x; c0[1] = -y; c0[2] = -1;
    c1[0] = +x; c1[1] = -y; c1[2] = -1;
    c2[0] = -x; c2[1] = +y; c2[2] = -1;
    c3[0] = +x; c3[1] = +y; c3[2] = -1;
}

void app::frustum::calc_corner_1(double *d, const double *a,
                                            const double *b,
                                            const double *c)
{
    // Compute a fourth screen corner given three.

    d[0] = b[0] + c[0] - a[0];
    d[1] = b[1] + c[1] - a[1];
    d[2] = b[2] + c[2] - a[2];
}

void app::frustum::calc_calibrated()
{
    bool b[4] = { false, false, false, false };

    double aspect =  1.333;
    double fov    = 90.000;

    double T[16], c[4][3];

    // Extract the frustum definition from the serialization node.

    if (node)
    {
        app::node curr;

        // Extract the screen corners.

        for (curr = find(node,       "corner"); curr;
             curr = next(node, curr, "corner"))
        {
            double *v = 0;

            // Determine which corner is being specified.

            if (const char *name = get_attr_s(curr, "name"))
            {
                if      (strcmp(name, "BL") == 0) { v = c[0]; b[0] = true; }
                else if (strcmp(name, "BR") == 0) { v = c[1]; b[1] = true; }
                else if (strcmp(name, "TL") == 0) { v = c[2]; b[2] = true; }
                else if (strcmp(name, "TR") == 0) { v = c[3]; b[3] = true; }
            }

            if (v)
            {
                // Extract the position.

                v[0] = get_attr_f(curr, "x");
                v[1] = get_attr_f(curr, "y");
                v[2] = get_attr_f(curr, "z");

                // Convert dimensions if necessary.

                if (const char *dim = get_attr_s(curr, "dim"))
                {
                    if (strcmp(dim, "mm") == 0)
                    {
                        v[0] /= 304.8;
                        v[1] /= 304.8;
                        v[2] /= 304.8;
                    }
                }
            }
        }

        // Extract field-of-view and aspect ratio.

        if ((curr = find(node, "perspective")))
        {
            aspect = get_attr_f(curr, "aspect");
            fov    = get_attr_f(curr, "fov");
        }

        // Extract the calibration.

        get_calibration(T);
    }
    else load_idt(T);

    // Compute any unspecified screen corner.

    if (!b[0] &&  b[1] &&  b[2] &&  b[3]) calc_corner_1(c[0], c[3], c[2], c[1]);
    if ( b[0] && !b[1] &&  b[2] &&  b[3]) calc_corner_1(c[1], c[2], c[0], c[3]);
    if ( b[0] &&  b[1] && !b[2] &&  b[3]) calc_corner_1(c[2], c[1], c[3], c[0]);
    if ( b[0] &&  b[1] &&  b[2] && !b[3]) calc_corner_1(c[3], c[0], c[1], c[2]);
    if (!b[0] && !b[1] && !b[2] && !b[3]) calc_corner_4(c[0], c[1], c[2], c[3],
                                                        aspect, fov);

    // Apply the calibration transform to the configured frustum corners.

    mult_mat_vec3(user_points[0], T, c[0]);
    mult_mat_vec3(user_points[1], T, c[1]);
    mult_mat_vec3(user_points[2], T, c[2]);
    mult_mat_vec3(user_points[3], T, c[3]);
}

void app::frustum::calc_user_planes(const double *p)
{
    user_pos[0] = p[0];
    user_pos[1] = p[1];
    user_pos[2] = p[2];

    // Compute the user-space view frustum bounding planes.

    set_plane(user_planes[0], p, user_points[0], user_points[2]);  // Left
    set_plane(user_planes[1], p, user_points[3], user_points[1]);  // Right
    set_plane(user_planes[2], p, user_points[1], user_points[0]);  // Bottom
    set_plane(user_planes[3], p, user_points[2], user_points[3]);  // Top

    // Cache the distance from the user to the display plane.

    double display_plane[4];

    set_plane(display_plane, user_points[0], user_points[1], user_points[2]);

    user_dist = DOT3(display_plane, p) + display_plane[3];
}

void app::frustum::calc_view_planes(const double *M,
                                    const double *I)
{
    mult_mat_vec3(view_pos, M, user_pos);

    // Compute the world-space view frustum bounding planes.

    mult_xps_vec4(view_planes[0], I, user_planes[0]);
    mult_xps_vec4(view_planes[1], I, user_planes[1]);
    mult_xps_vec4(view_planes[2], I, user_planes[2]);
    mult_xps_vec4(view_planes[3], I, user_planes[3]);
}

void app::frustum::calc_view_points(double n, double f)
{
    double v[4][3];

    // Compute the world-space frustum corner vectors.

    crossprod(v[0], view_planes[2], view_planes[0]);
    crossprod(v[1], view_planes[1], view_planes[2]);
    crossprod(v[2], view_planes[0], view_planes[3]);
    crossprod(v[3], view_planes[3], view_planes[1]);
    
    normalize(v[0]);
    normalize(v[1]);
    normalize(v[2]);
    normalize(v[3]);

    // Compute the world-space view frustum points.

    for (int i = 0; i < 4; ++i)
    {
        view_points[i + 0][0] = view_pos[0] + v[i][0] * n / user_dist;
        view_points[i + 0][1] = view_pos[1] + v[i][1] * n / user_dist;
        view_points[i + 0][2] = view_pos[2] + v[i][2] * n / user_dist;

        view_points[i + 4][0] = view_pos[0] + v[i][0] * f / user_dist;
        view_points[i + 4][1] = view_pos[1] + v[i][1] * f / user_dist;
        view_points[i + 4][2] = view_pos[2] + v[i][2] * f / user_dist;
    }
}

void app::frustum::calc_projection(double n, double f)
{
    // Compute the display plane basis.

    double B[16];

    load_idt(B);

    B[0] = user_points[1][0] - user_points[0][0];
    B[1] = user_points[1][1] - user_points[0][1];
    B[2] = user_points[1][2] - user_points[0][2];

    B[4] = user_points[2][0] - user_points[0][0];
    B[5] = user_points[2][1] - user_points[0][1];
    B[6] = user_points[2][2] - user_points[0][2];

    normalize(B + 0);
    normalize(B + 4);
    crossprod(B + 8, B + 0, B + 4);
    normalize(B + 8);

    // Compute the screen corner vectors.

    double v[4][4];

    for (int i = 0; i < 4; ++i)
    {
        v[i][0] = user_points[i][0] - user_pos[0];
        v[i][1] = user_points[i][1] - user_pos[1];
        v[i][2] = user_points[i][2] - user_pos[2];
    }

    // Generate the off-axis projection.

    double l = DOT3(B + 0, v[0]) * n / user_dist;
    double r = DOT3(B + 0, v[1]) * n / user_dist;
    double b = DOT3(B + 4, v[0]) * n / user_dist;
    double t = DOT3(B + 4, v[2]) * n / user_dist;

    load_persp(P, l, r, b, t, n, f);

    // Orient the projection and move the apex to the origin.

    mult_mat_mat(P, P, B);
    Rmul_xlt_inv(P, user_pos[0],
                    user_pos[1],
                    user_pos[2]);
}

//-----------------------------------------------------------------------------

app::frustum::frustum(app::node node) : node(node), user_dist(1.0)
{
    user_pos[0] = 0.0;
    user_pos[1] = 0.0;
    user_pos[2] = 0.0;

    calc_calibrated();
}

app::frustum::frustum(frustum& that) : node(0)
{
    // Copy the user-space data.

    memcpy(user_pos, that.user_pos, 3 * sizeof (double));

    user_dist = that.user_dist;

    memcpy(user_points, that.user_points, 4 * 3 * sizeof (double));
    memcpy(user_planes, that.user_planes, 4 * 4 * sizeof (double));

    memcpy(P, that.P, 16 * sizeof (double));
}

//-----------------------------------------------------------------------------

bool app::frustum::input_point(int i, const double *p, const double *q)
{
    return false;
}

bool app::frustum::input_click(int i, int b, int m, bool d)
{
    return false;
}

bool app::frustum::input_keybd(int c, int k, int m, bool d)
{
    return false;
}

//-----------------------------------------------------------------------------

static double closest_point(const double *a, double ar,
                            const double *b, double br)
{
    double d[3];
    double p[3];

    d[0] = b[0] - a[0];
    d[1] = b[1] - a[1];
    d[2] = b[2] - a[2];

    normalize(d);

    double da;
    double db;

    if ((da = DOT3(d, a)) >= 0.0) return ar;
    if ((db = DOT3(d, b)) <= 0.0) return br;

    p[0] = a[0] - d[0] * da;
    p[1] = a[1] - d[1] * da;
    p[2] = a[2] - d[2] * da;

    return sqrt(DOT3(p, p));
}

static bool test_shell_vector(const double *n0,
                              const double *n1,
                              const double *n2,
                              const double *v)
{
    double n[3];

    crossprod(n, n0, n1);

    if (DOT3(n, v) < 0.0)
        return false;

    crossprod(n, n1, n2);

    if (DOT3(n, v) < 0.0)
        return false;

    crossprod(n, n2, n0);

    if (DOT3(n, v) < 0.0)
        return false;

    return true;
}

static int test_shell_plane(const double *n0,
                            const double *n1,
                            const double *n2,
                            const double *P, double r0, double r1)
{
    double R0;
    double R1;

    // Easy-out the total misses.

    const double d  = P[3];
    const double d0 = DOT3(n0, P);
    const double d1 = DOT3(n1, P);
    const double d2 = DOT3(n2, P);

    if (d <  0 && d0 <  0 && d1 <  0 && d2 <  0) return -1;
    if (d >= 0 && d0 >= 0 && d1 >= 0 && d2 >= 0) return +1;

    // Compute the vector-plane intersection distances.

    const double l0 = -d / d0;
    const double l1 = -d / d1;
    const double l2 = -d / d2;

    // Hyperbolic: max is infinity. Elliptic: one of the points is max.

    if ((d0 <= 0 && d1 <= 0 && d2 <= 0) ||
        (d0 >  0 && d1 >  0 && d2 >  0))
    {
        R1 = std::max(l0, l1);
        R1 = std::max(R1, l2);
    }
    else
    {
        R1 = std::numeric_limits<double>::max();
    }

    if (test_shell_vector(n0, n1, n2, P))
    {
        // If the normal falls within the triangle, the normal has min radius.

        R0 = -d;
    }
    else
    {
        // Otherwise, min radius is on an edge.

        double p0[3];
        double p1[3];
        double p2[3];

        R0 = std::numeric_limits<double>::max();

        if (l0 >= 0)
        {
            p0[0] = n0[0] * l0;
            p0[1] = n0[1] * l0;
            p0[2] = n0[2] * l0;
            R0 = std::min(R0, l0);
        }

        if (l1 >= 0)
        {
            p1[0] = n1[0] * l1;
            p1[1] = n1[1] * l1;
            p1[2] = n1[2] * l1;
            R0 = std::min(R0, l1);
        }

        if (l2 >= 0)
        {
            p2[0] = n2[0] * l2;
            p2[1] = n2[1] * l2;
            p2[2] = n2[2] * l2;
            R0 = std::min(R0, l2);
        }

        if (l0 >= 0 && l1 >= 0) R0 =std::min(R0, closest_point(p0, l0, p1, l1));
        if (l1 >= 0 && l2 >= 0) R0 =std::min(R0, closest_point(p1, l1, p2, l2));
        if (l2 >= 0 && l0 >= 0) R0 =std::min(R0, closest_point(p2, l2, p0, l0));
    }

    // Interpret the computed radii as hit or miss.

    if (d > 0)
    {
        if (R1 < r0) return -1;
        if (R0 > r1) return +1;
    }
    else
    {
        if (R0 > r1) return -1;
        if (R1 < r0) return +1;
    }
    return 0;
}

int app::frustum::test_shell(const double *n0,
                             const double *n1,
                             const double *n2,
                             const double *hp, double r0, double r1) const
{
    int i, d, c = 0;

    // Test the horizon plane.

    if ((d = test_shell_plane(n0, n1, n2, hp, r0, r1)) < 0)
        return -1;
    else
        c += d;

    // Test the four planes of this frustum.

    for (i = 0; i < 4; ++i)
        if ((d = test_shell_plane(n0, n1, n2, view_planes[i], r0, r1)) < 0)
            return -1;
        else
            c += d;

    // If all planes pass fully, return short-circuiting pass.

    return (c == 5) ? 1 : 0;

}

//-----------------------------------------------------------------------------

void app::frustum::draw() const
{
    glMatrixMode(GL_PROJECTION);
    {
        glLoadIdentity();
        glMultMatrixd(P);
    }
    glMatrixMode(GL_MODELVIEW);
}

//-----------------------------------------------------------------------------
