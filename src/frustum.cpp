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

void app::frustum::calc_corner_4(double aspect, double fov)
{
    // Compute screen corners given perspective field-of-view and aspect ratio.

    const double x = tan(RAD(fov * 0.5));
    const double y = x / aspect;

    c[0][0] = -x; c[0][1] = -y; c[0][2] = -1;
    c[1][0] = +x; c[1][1] = -y; c[1][2] = -1;
    c[2][0] = -x; c[2][1] = +y; c[2][2] = -1;
    c[3][0] = +x; c[3][1] = +y; c[3][2] = -1;
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
    // Apply the calibration transform to the configured frustum corners.

    mult_mat_vec3(C[0], T, c[0]);
    mult_mat_vec3(C[1], T, c[1]);
    mult_mat_vec3(C[2], T, c[2]);
    mult_mat_vec3(C[3], T, c[3]);
}

void app::frustum::calc_user_planes(const double *p)
{
    // Compute the user-space view frustum bounding planes.

    set_plane(user_planes[0], p, C[0], C[2]);  // Left
    set_plane(user_planes[1], p, C[3], C[1]);  // Right
    set_plane(user_planes[2], p, C[1], C[0]);  // Bottom
    set_plane(user_planes[3], p, C[2], C[3]);  // Top
}

void app::frustum::calc_view_planes(const double *I)
{
    // Compute the world-space view frustum bounding planes.

    mult_xps_vec4(view_planes[0], I, user_planes[0]);
    mult_xps_vec4(view_planes[1], I, user_planes[1]);
    mult_xps_vec4(view_planes[2], I, user_planes[2]);
    mult_xps_vec4(view_planes[3], I, user_planes[3]);
}

void app::frustum::calc_projection(const double *p,
                                   const double *M, double n, double f)
{
    // Compute the display plane basis.

    double B[16], x[4], y[4];

    load_idt(B);

    B[0] = C[1][0] - C[0][0];
    B[1] = C[1][1] - C[0][1];
    B[2] = C[1][2] - C[0][2];

    B[4] = C[2][0] - C[0][0];
    B[5] = C[2][1] - C[0][1];
    B[6] = C[2][2] - C[0][2];

    normalize(B + 0);
    normalize(B + 4);
    crossprod(B + 8, B + 0, B + 4);
    normalize(B + 8);

    // Compute the distance to the display plane.

    const double dd = DOT3(B + 8, p) - DOT3(B + 8, C[0]);
    const double nk = n / dd;
    const double fk = f / dd;

    // For each frustum corner...

    for (int i = 0; i < 4; ++i)
    {
        double v[3];
        double q[3];

        // Compute the current frustum corner vector.

        v[0] = C[i][0] - p[0];
        v[1] = C[i][1] - p[1];
        v[2] = C[i][2] - p[2];

        // Compute the world-space far position.

        q[0] = v[0] * fk;
        q[1] = v[1] * fk;
        q[2] = v[2] * fk;

        mult_mat_vec3(view_planes[i + 4], M, q);

        // Compute the world-space near position.

        q[0] = v[0] * nk;
        q[1] = v[1] * nk;
        q[2] = v[2] * nk;

        mult_mat_vec3(view_planes[i + 0], M, q);

        // Note the display plane extents.

        x[i] = DOT3(B + 0, q);
        y[i] = DOT3(B + 4, q);
    }

    // Generate the off-axis projection.

    load_persp(P, x[0], x[1], y[0], y[2], n, f);

    // Orient the projection and move the apex to the origin.

    mult_mat_mat(P, P, B);
    Rmul_xlt_inv(P, p[0], p[1], p[2]);
}

//-----------------------------------------------------------------------------

app::frustum::frustum(mxml_node_t *node) : node(node)
{
    bool b[4] = { false, false, false, false };

    double aspect =  1.333;
    double fov    = 90.000;

    load_idt(T);

    if (node)
    {
        mxml_node_t *curr;

        // Extract the screen corners.

        MXML_FORALL(node, curr, "corner")
        {
            double *v = 0;

            // Determine which corner is being specified.

            if (const char *name = mxmlElementGetAttr(curr, "name"))
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

                if (const char *dim = mxmlElementGetAttr(curr, "dim"))
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

        if ((curr = mxmlFindElement(node, node, "perspective",
                                    0, 0, MXML_DESCEND)))
        {
            aspect = get_attr_f(curr, "aspect");
            fov    = get_attr_f(curr, "fov");
        }

        // Extract the calibration matrix.

        if ((curr = mxmlFindElement(node, node, "calibration",
                                    0, 0, MXML_DESCEND)))
        {
            T[ 0] = get_attr_f(curr, "m0");
            T[ 1] = get_attr_f(curr, "m1");
            T[ 2] = get_attr_f(curr, "m2");
            T[ 3] = get_attr_f(curr, "m3");
            T[ 4] = get_attr_f(curr, "m4");
            T[ 5] = get_attr_f(curr, "m5");
            T[ 6] = get_attr_f(curr, "m6");
            T[ 7] = get_attr_f(curr, "m7");
            T[ 8] = get_attr_f(curr, "m8");
            T[ 9] = get_attr_f(curr, "m9");
            T[10] = get_attr_f(curr, "mA");
            T[11] = get_attr_f(curr, "mB");
            T[12] = get_attr_f(curr, "mC");
            T[13] = get_attr_f(curr, "mD");
            T[14] = get_attr_f(curr, "mE");
            T[15] = get_attr_f(curr, "mF");
        }
    }

    // Compute any unspecified screen corner.

    if (!b[0] &&  b[1] &&  b[2] &&  b[3]) calc_corner_1(C[0], C[3], C[2], C[1]);
    if ( b[0] && !b[1] &&  b[2] &&  b[3]) calc_corner_1(C[1], C[2], C[0], C[3]);
    if ( b[0] &&  b[1] && !b[2] &&  b[3]) calc_corner_1(C[2], C[1], C[3], C[0]);
    if ( b[0] &&  b[1] &&  b[2] && !b[3]) calc_corner_1(C[3], C[0], C[1], C[2]);
    if (!b[0] && !b[1] && !b[2] && !b[3]) calc_corner_4(aspect, fov);

    calc_calibrated();
}

//-----------------------------------------------------------------------------

void app::frustum::set_view(const double *p,
                            const double *I)
{
    // Cache the frustum bounding planes.

    calc_user_planes(p);
    calc_view_planes(I);
}

void app::frustum::set_dist(const double *p,
                            const double *M, double n, double f)
{
    // Cache the frustum bounding points and the projection transform.

    calc_projection(p, M, n, f);
}

//-----------------------------------------------------------------------------

bool app::frustum::input_point(double x, double y)
{
    return false;
}

bool app::frustum::input_click(int b, int m, bool d)
{
    return false;
}

bool app::frustum::input_keybd(int k, int m, bool d)
{
    return false;
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
