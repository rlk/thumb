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

#include <SDL_keyboard.h>

#include "app-frustum.hpp"
#include "app-event.hpp"
#include "ogl-opengl.hpp"
#include "matrix.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

void app::frustum::get_calibration(double& P, double& T, double& R,
                                   double& p, double& y, double& r,
                                   double& H, double& V)
{
    // Assign defaults for any undefined parameters.

    P =  0.0;  // Position phi
    T =  0.0;  // Position theta
    R =  0.0;  // Position rho
    p =  0.0;  // Rotation pitch
    y =  0.0;  // Rotation yaw
    r =  0.0;  // Rotation roll
    H = 60.0;  // Horizontal field of view
    V = 45.0;  // Vertical   field of view

    // Extract the calibration from the serialization node.

    if (node)
    {
        app::node curr;

        if ((curr = find(node, "position")))
        {
            P = get_attr_f(curr, "p", P);
            T = get_attr_f(curr, "t", T);
            R = get_attr_f(curr, "r", R);
        }
        if ((curr = find(node, "rotation")))
        {
            p = get_attr_f(curr, "p", p);
            y = get_attr_f(curr, "y", y);
            r = get_attr_f(curr, "r", r);
        }
        if ((curr = find(node, "perspective")))
        {
            H = get_attr_f(curr, "hfov", H);
            V = get_attr_f(curr, "vfov", V);
        }
    }
}

void app::frustum::set_calibration(double P, double T, double R,
                                   double p, double y, double r,
                                   double H, double V)
{
    // Update the calibration in the serialization node.

    if (node)
    {
        app::node curr;

        if ((curr = find(node, "position")))
        {
            set_attr_f(curr, "p", P);
            set_attr_f(curr, "t", T);
            set_attr_f(curr, "r", R);
        }
        if ((curr = find(node, "rotation")))
        {
            set_attr_f(curr, "p", p);
            set_attr_f(curr, "y", y);
            set_attr_f(curr, "r", r);
        }
        if ((curr = find(node, "perspective")))
        {
            set_attr_f(curr, "hfov", H);
            set_attr_f(curr, "vfov", V);
        }
    }
}

void app::frustum::mat_calibration(double *M)
{
    // Convert the current calibration to a transformation matrix.

    double P, T, R, p, y, r, H, V;

    get_calibration(P, T, R, p, y, r, H, V);

    load_idt(M);

    Rmul_rot_mat(M, 0, 1, 0, T);
    Rmul_rot_mat(M, 1, 0, 0, P);
    Rmul_xlt_mat(M, 0, 0, R);

    Rmul_rot_mat(M, 0, 1, 0, y);
    Rmul_rot_mat(M, 1, 0, 0, p);
    Rmul_rot_mat(M, 0, 0, 1, r);
}

//-----------------------------------------------------------------------------

void app::frustum::calc_corner_4(double *c0,
                                 double *c1,
                                 double *c2,
                                 double *c3, double H, double V)
{
    // Compute screen corners given perspective fields-of-view.

    const double x = tan(RAD(H * 0.5));
    const double y = tan(RAD(V * 0.5));

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
    // Extract the frustum definition from the serialization node.

    bool b[4] = { false, false, false, false };

    double hfov = 60.000;
    double vfov = 45.000;

    double c[4][3];

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

        // Extract fields-of-view.

        if ((curr = find(node, "perspective")))
        {
            hfov = get_attr_f(curr, "hfov");
            vfov = get_attr_f(curr, "vfov");
        }

        // Extract the calibration.

        mat_calibration(T);
    }
    else load_idt(T);

    // Compute any unspecified screen corner.

    if (!b[0] &&  b[1] &&  b[2] &&  b[3]) calc_corner_1(c[0],c[3],c[2],c[1]);
    if ( b[0] && !b[1] &&  b[2] &&  b[3]) calc_corner_1(c[1],c[2],c[0],c[3]);
    if ( b[0] &&  b[1] && !b[2] &&  b[3]) calc_corner_1(c[2],c[1],c[3],c[0]);
    if ( b[0] &&  b[1] &&  b[2] && !b[3]) calc_corner_1(c[3],c[0],c[1],c[2]);
    if (!b[0] && !b[1] && !b[2] && !b[3]) calc_corner_4(c[0],c[1],c[2],c[3],
                                                        hfov, vfov);

    // Apply the calibration transform to the configured frustum corners.

    mult_mat_vec3(user_points[0], T, c[0]);
    mult_mat_vec3(user_points[1], T, c[1]);
    mult_mat_vec3(user_points[2], T, c[2]);
    mult_mat_vec3(user_points[3], T, c[3]);

    // Cache the display basis.

    load_idt(user_basis);

    user_basis[0] = user_points[1][0] - user_points[0][0];
    user_basis[1] = user_points[1][1] - user_points[0][1];
    user_basis[2] = user_points[1][2] - user_points[0][2];

    user_basis[4] = user_points[2][0] - user_points[0][0];
    user_basis[5] = user_points[2][1] - user_points[0][1];
    user_basis[6] = user_points[2][2] - user_points[0][2];

    normalize(user_basis + 0);
    normalize(user_basis + 4);
    crossprod(user_basis + 8, user_basis + 0, user_basis + 4);
    normalize(user_basis + 8);
}

void app::frustum::calc_user_planes(const double *p)
{
    // Compute the calibrated user position, and display-space position.

    mult_mat_vec3(user_pos, T, p);

    // Compute the vector from the screen center to the viewer.

    double v[3];

    v[0] = user_pos[0] - (user_points[0][0] + user_points[3][0]) * 0.5;
    v[1] = user_pos[1] - (user_points[0][1] + user_points[3][1]) * 0.5;
    v[2] = user_pos[2] - (user_points[0][2] + user_points[3][2]) * 0.5;

    mult_xps_vec3(disp_pos, user_basis, v);

    // Compute the user-space view frustum bounding planes.

    set_plane(user_planes[0], user_pos, user_points[0], user_points[2]); // L
    set_plane(user_planes[1], user_pos, user_points[3], user_points[1]); // R
    set_plane(user_planes[2], user_pos, user_points[1], user_points[0]); // B
    set_plane(user_planes[3], user_pos, user_points[2], user_points[3]); // T

    // Cache the solid angle of the frustum.

    double u[4][3];

    for (int i = 0; i < 4; ++i)
    {
        u[i][0] = user_points[i][0] - user_pos[0];
        u[i][1] = user_points[i][1] - user_pos[1];
        u[i][2] = user_points[i][2] - user_pos[2];

        normalize(u[i]);
    }

    user_angle = (solid_angle(u[0], u[2], u[1]) +
                  solid_angle(u[1], u[2], u[3]));

    // Cache the distance from the user to the display plane.

    double display_plane[4];

    set_plane(display_plane, user_points[0], user_points[1], user_points[2]);

    user_dist = DOT3(display_plane, user_pos) + display_plane[3];
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

    view_count = 4;
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
    // Compute the screen corner vectors.

    double v[4][4];

    for (int i = 0; i < 4; ++i)
    {
        v[i][0] = user_points[i][0] - user_pos[0];
        v[i][1] = user_points[i][1] - user_pos[1];
        v[i][2] = user_points[i][2] - user_pos[2];
    }

    // Generate the off-axis projection.

    double l = DOT3(user_basis + 0, v[0]) * n / user_dist;
    double r = DOT3(user_basis + 0, v[1]) * n / user_dist;
    double b = DOT3(user_basis + 4, v[0]) * n / user_dist;
    double t = DOT3(user_basis + 4, v[2]) * n / user_dist;

    load_persp(P, l, r, b, t, n, f);

    // Orient the projection and move the apex to the origin.

    double A[16];

    load_xps(A, user_basis);

    mult_mat_mat(P, P, A);
    Rmul_xlt_inv(P, user_pos[0],
                    user_pos[1],
                    user_pos[2]);
}

void app::frustum::set_horizon(double r)
{
    // Use the view position and given radius to compute the horizon plane.

    view_planes[4][0] = view_pos[0];
    view_planes[4][1] = view_pos[1];
    view_planes[4][2] = view_pos[2];

    normalize(view_planes[4]);

    view_planes[4][3] = -r * r / sqrt(DOT3(view_pos, view_pos));

    view_count = 5;
}

double app::frustum::get_w() const
{
    // Compute and return the physical width of the display.

    double d[3];

    d[0] = user_points[1][0] - user_points[0][0];
    d[1] = user_points[1][1] - user_points[0][1];
    d[2] = user_points[1][2] - user_points[0][2];

    return sqrt(DOT3(d, d));
}

double app::frustum::get_h() const
{
    // Compute and return the physical height of the display.

    double d[3];

    d[0] = user_points[2][0] - user_points[0][0];
    d[1] = user_points[2][1] - user_points[0][1];
    d[2] = user_points[2][2] - user_points[0][2];

    return sqrt(DOT3(d, d));
}

double app::frustum::pixels(double angle) const
{
    // Estimate and return the number of pixels in the given solid angle.

    return pixel_w * pixel_h * angle / user_angle;
}

//-----------------------------------------------------------------------------

app::frustum::frustum(app::node node, int w, int h)
    : node(node), user_dist(1.0), view_count(0), pixel_w(w), pixel_h(h)
{
    user_pos[0] = 0.0;
    user_pos[1] = 0.0;
    user_pos[2] = 0.0;

    calc_calibrated();
}

app::frustum::frustum(frustum& that)
    : node(0), view_count(0)
{
    // Copy the user-space data.

    memcpy(user_pos, that.user_pos, 3 * sizeof (double));

    pixel_w = that.pixel_w;
    pixel_h = that.pixel_h;

    user_angle = that.user_angle;
    user_dist  = that.user_dist;

    memcpy(user_points, that.user_points, 4 * 3 * sizeof (double));
    memcpy(user_planes, that.user_planes, 4 * 4 * sizeof (double));
    memcpy(user_basis,  that.user_basis,     16 * sizeof (double));

    memcpy(P, that.P, 16 * sizeof (double));
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

        if (l0 >= 0 && l1 >= 0) R0=std::min(R0, closest_point(p0, l0, p1, l1));
        if (l1 >= 0 && l2 >= 0) R0=std::min(R0, closest_point(p1, l1, p2, l2));
        if (l2 >= 0 && l0 >= 0) R0=std::min(R0, closest_point(p2, l2, p0, l0));
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

static bool test_shell_point(const double *p,
                             const double *n0,
                             const double *n1,
                             const double *n2, double r0, double r1)
{
    double rr = DOT3(p, p);

    if (r0 * r0 < rr && rr < r1 * r1)
    {
        double n[3];

        crossprod(n, n0, n1); if (DOT3(n, p) < 0.0) return false; 
        crossprod(n, n1, n2); if (DOT3(n, p) < 0.0) return false; 
        crossprod(n, n2, n0); if (DOT3(n, p) < 0.0) return false;

        return true;
    }
    return false;
}

int app::frustum::test_shell(const double *n0,
                             const double *n1,
                             const double *n2, double r0, double r1) const
{
    int i, d, c = 0;

    // If the viewpoint is within the shell, conservative pass.

    if (test_shell_point(view_pos, n0, n1, n2, r0, r1))
        return 0;

    // Test the planes of this frustum.

    for (i = view_count - 1; i >= 0; --i)
        if ((d = test_shell_plane(n0, n1, n2, view_planes[i], r0, r1)) < 0)
            return -1;
        else
            c += d;

    // If all planes pass fully, return short-circuiting pass.

    return (c == view_count) ? 1 : 0;
}

//-----------------------------------------------------------------------------

static int test_cap_plane(const double *n, double a,
                          const double *P, double r0, double r1)
{
    // Short circuit a total miss of the sphere.

    if (-P[3] > r1) return -1;
    if ( P[3] > r1) return +1;

    // An intersection has occurred so there's work to do.

    double a0 = acos(-P[3] / r0);
    double a1 = acos(-P[3] / r1);
    double am = std::max(a0, a1);

    double d = acos(DOT3(n, P));

    if (d + a < am) return +1;
    if (d - a > am) return -1;

    return 0;
}

int app::frustum::test_cap(const double *n, double a,
                                 double r0, double r1) const
{
    int i, d, c = 0;

    // Test the planes of this frustum.

    for (i = view_count - 1; i >= 0; --i)
        if ((d = test_cap_plane(n, a, view_planes[i], r0, r1)) < 0)
            return -1;
        else
            c += d;

    // If all planes pass fully, return short-circuiting pass.

    return (c == view_count) ? 1 : 0;
}

//-----------------------------------------------------------------------------

bool app::frustum::pointer_to_2D(event *E, int& x, int& y) const
{
    double M[16], V[3], plane[4], point[3];

    // Determine the pointer vector from the quaternion.

    set_quaternion(M, E->data.point.q);

    V[0] = -M[ 8];
    V[1] = -M[ 9];
    V[2] = -M[10];

    // Find the plane of the display.

    set_plane(plane, user_points[0],
                     user_points[1],
                     user_points[2]);

    // Compute the point of intersection with the display plane.

    double t = -(plane[3] + DOT3(E->data.point.p, plane)) / DOT3(V, plane);

    point[0] = E->data.point.p[0] + V[0] * t - user_points[0][0];
    point[1] = E->data.point.p[1] + V[1] * t - user_points[0][1];
    point[2] = E->data.point.p[2] + V[2] * t - user_points[0][2];

    // Project this point into screen space.

    double R[3];
    double U[3];

    R[0] = user_points[1][0] - user_points[0][0];
    R[1] = user_points[1][1] - user_points[0][1];
    R[2] = user_points[1][2] - user_points[0][2];

    U[0] = user_points[2][0] - user_points[0][0];
    U[1] = user_points[2][1] - user_points[0][1];
    U[2] = user_points[2][2] - user_points[0][2];

    normalize(R);
    normalize(U);

    double u = DOT3(point, R);
    double v = DOT3(point, U);

    // Confirm that the point falls within the frustum.

    if (0 < u && u < 1 && 0 < v && v < 1)
    {
        x = nearest_int(u * double(pixel_w) / get_w());
        y = nearest_int(v * double(pixel_h) / get_h());
        return true;
    }
    return false;
}

bool app::frustum::pointer_to_3D(event *E, int x, int y) const
{
    double X = double(x) / double(pixel_w);
    double Y = double(y) / double(pixel_h);

    // Return the point event for (x, y) in the current user space.

    double B[16], k = 1.0 - X - Y;

    load_idt(B);

    // Compute the Z axis of the pointer space.

    B[ 8] = user_pos[0] - (user_points[0][0] * k +
                           user_points[1][0] * X +
                           user_points[2][0] * Y);
    B[ 9] = user_pos[1] - (user_points[0][1] * k +
                           user_points[1][1] * X +
                           user_points[2][1] * Y);
    B[10] = user_pos[2] - (user_points[0][2] * k +
                           user_points[1][2] * X +
                           user_points[2][2] * Y);

    // Complete an orthonormal basis of the pointer space.

    normalize(B + 8);
    crossprod(B + 0, B + 4, B + 8);
    normalize(B + 0);
    crossprod(B + 4, B + 8, B + 0);
    normalize(B + 4);

    // Convert the pointer space basis matrix to a quaternion.

    double q[3];

    get_quaternion(q, B);

    // Store the pointer origin and direction in the event.

    E->mk_point(0, user_pos, q);

    return true;
}

bool app::frustum::process_event(app::event *E)
{
    if (E->get_type() == E_KEYBD && E->data.keybd.d)
    {
        const int k = E->data.keybd.k;
        const int m = E->data.keybd.m;

        if (m & KMOD_CTRL)
        {
            double d = ((m & KMOD_CAPS) || (m & KMOD_ALT)) ? 0.05 : 0.5;

            double dP = 0;
            double dT = 0;
            double dR = 0;
            double dp = 0;
            double dy = 0;
            double dr = 0;
            double dH = 0;
            double dV = 0;

            bool b = false;

            // Interpret the key event.

            if (m & KMOD_SHIFT)
            {
                if      (k == SDLK_LEFT)     { dT =  d; b = true; }
                else if (k == SDLK_RIGHT)    { dT = -d; b = true; }
                else if (k == SDLK_UP)       { dP =  d; b = true; }
                else if (k == SDLK_DOWN)     { dP = -d; b = true; }
                else if (k == SDLK_PAGEUP)   { dR =  d; b = true; }
                else if (k == SDLK_PAGEDOWN) { dR = -d; b = true; }
            }
            else
            {
                if      (k == SDLK_LEFT)     { dy = -d; b = true; }
                else if (k == SDLK_RIGHT)    { dy =  d; b = true; }
                else if (k == SDLK_UP)       { dp = -d; b = true; }
                else if (k == SDLK_DOWN)     { dp =  d; b = true; }
                else if (k == SDLK_PAGEUP)   { dr =  d; b = true; }
                else if (k == SDLK_PAGEDOWN) { dr = -d; b = true; }
                else if (k == SDLK_INSERT)   { dH =  d; b = true; }
                else if (k == SDLK_DELETE)   { dH = -d; b = true; }
                else if (k == SDLK_HOME)     { dV =  d; b = true; }
                else if (k == SDLK_END)      { dV = -d; b = true; }
            }

            // If changes occurred, apply them to the calibration.

            if (b)
            {
                double P, T, R, p, y, r, H, V;

                get_calibration(P, T, R, p, y, r, H, V);
                P += dP;
                T += dT;
                R += dR;
                p += dp;
                y += dy;
                r += dr;
                H += dH;
                V += dV;
                set_calibration(P, T, R, p, y, r, H, V);
                calc_calibrated();

                return true;
            }
        }
    }
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

void app::frustum::cast() const
{
    glBegin(GL_QUADS);
    {
        glTexCoord3dv(user_points[0]);
        glVertex2f(-1.0f, -1.0f);
        glTexCoord3dv(user_points[1]);
        glVertex2f(+1.0f, -1.0f);
        glTexCoord3dv(user_points[3]);
        glVertex2f(+1.0f, +1.0f);
        glTexCoord3dv(user_points[2]);
        glVertex2f(-1.0f, +1.0f);
    }
    glEnd();
}

void app::frustum::rect() const
{
    glBegin(GL_QUADS);
    {
        glVertex3dv(user_points[0]);
        glVertex3dv(user_points[1]);
        glVertex3dv(user_points[3]);
        glVertex3dv(user_points[2]);
    }
    glEnd();
}

//-----------------------------------------------------------------------------
