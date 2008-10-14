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

#include <SDL_mouse.h>
#include <SDL_keyboard.h>

#include "app-frustum.hpp"
#include "ogl-opengl.hpp"
#include "matrix.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

void app::frustum::get_calibration(double& P, double& T, double& R,
                                   double& p, double& y, double& r,
                                   double& F, double& A)
{
    P =  0.0;  // Position phi
    T =  0.0;  // Position theta
    R =  0.0;  // Position rho
    p =  0.0;  // Rotation pitch
    y =  0.0;  // Rotation yaw
    r =  0.0;  // Rotation roll
    F = 60.0;  // Field of view
    A =  1.33; // Aspect ratio

    // Extract the calibration matrix from the serialization node.

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
            A = get_attr_f(curr, "aspect", A);
            F = get_attr_f(curr, "fov",    F);
        }
    }
}

void app::frustum::set_calibration(double P, double T, double R,
                                   double p, double y, double r,
                                   double F, double A)
{
    // Update the calibration matrix in the serialization node.

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
            set_attr_f(curr, "aspect", A);
            set_attr_f(curr, "fov",    F);
        }
    }
}

void app::frustum::mat_calibration(double *M)
{
    double P, T, R, p, y, r, F, A;

    get_calibration(P, T, R, p, y, r, F, A);

    load_rot_mat(M, 0, 1, 0, T);
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
    double fov    = 60.000;

    double c[4][3];

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

        mat_calibration(T);
    }
    else load_idt(T);

    // Compute any unspecified screen corner.

    if (!b[0] &&  b[1] &&  b[2] &&  b[3]) calc_corner_1(c[0],c[3],c[2],c[1]);
    if ( b[0] && !b[1] &&  b[2] &&  b[3]) calc_corner_1(c[1],c[2],c[0],c[3]);
    if ( b[0] &&  b[1] && !b[2] &&  b[3]) calc_corner_1(c[2],c[1],c[3],c[0]);
    if ( b[0] &&  b[1] &&  b[2] && !b[3]) calc_corner_1(c[3],c[0],c[1],c[2]);
    if (!b[0] && !b[1] && !b[2] && !b[3]) calc_corner_4(c[0],c[1],c[2],c[3],
                                                        aspect, fov);

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

/*
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
*/
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

void app::frustum::calc_dome_planes(const double *p,
                                    const frustum *proj,
                                    double rad,
                                    double t0, double t1,
                                    double p0, double p1)
{
    const double *o = proj->user_pos;

    double P[4][3], R[3], U[3], B[3], C[3] = { 0.0f, 0.0f, 0.0f };

    // Compute the points at which proj intersects the dome.
/*
    for (int i = 0; i < 4; ++i)
    {
        double v[3];

        v[0] = proj->user_points[i][0] - o[0];
        v[1] = proj->user_points[i][1] - o[1];
        v[2] = proj->user_points[i][2] - o[2];

        double a = DOT3(v, v);
        double b = DOT3(v, o) * 2.0;
        double c = DOT3(o, o) - rad * rad;

        double t = std::max((-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a),
                            (-b - sqrt(b * b - 4.0 * a * c)) / (2.0 * a));

        P[i][0] = o[0] + v[0] * t;
        P[i][1] = o[1] + v[1] * t;
        P[i][2] = o[2] + v[2] * t;

        C[0] += P[i][0];
        C[1] += P[i][1];
        C[2] += P[i][2];
    }

    C[0] *= 0.25;
    C[1] *= 0.25;
    C[2] *= 0.25;
    */

    P[0][0] =  rad * sin(RAD(t0)) * cos(RAD(p0));
    P[0][1] =  rad *                sin(RAD(p0));
    P[0][2] = -rad * cos(RAD(t0)) * cos(RAD(p0));

    P[1][0] =  rad * sin(RAD(t1)) * cos(RAD(p0));
    P[1][1] =  rad *                sin(RAD(p0));
    P[1][2] = -rad * cos(RAD(t1)) * cos(RAD(p0));

    P[2][0] =  rad * sin(RAD(t0)) * cos(RAD(p1));
    P[2][1] =  rad *                sin(RAD(p1));
    P[2][2] = -rad * cos(RAD(t0)) * cos(RAD(p1));

    P[3][0] =  rad * sin(RAD(t1)) * cos(RAD(p1));
    P[3][1] =  rad *                sin(RAD(p1));
    P[3][2] = -rad * cos(RAD(t1)) * cos(RAD(p1));

    C[0] = 0.25 * (P[0][0] + P[1][0] + P[2][0] + P[3][0]);
    C[1] = 0.25 * (P[0][1] + P[1][1] + P[2][1] + P[3][1]);
    C[2] = 0.25 * (P[0][2] + P[1][2] + P[2][2] + P[3][2]);

    /* Compute a frustum that encompasses these. */

    B[0] = p[0] - C[0];
    B[1] = p[1] - C[1];
    B[2] = p[2] - C[2];

    normalize(B);

    if (fabs(B[1]) > 0.9)
    {
        U[0] = 0.0;
        U[1] = 0.0;
        U[2] = 1.0;
    }
    else
    {
        U[0] = 0.0;
        U[1] = 1.0;
        U[2] = 0.0;
    }

    crossprod(R, U, B);
    normalize(R);
    crossprod(U, B, R);
    normalize(U);

    double x[4];
    double y[4];

    for (int i = 0; i < 4; ++i)
    {
        double V[3];

        V[0] = P[i][0] - p[0];
        V[1] = P[i][1] - p[1];
        V[2] = P[i][2] - p[2];

        x[i] = DOT3(V, R);
        y[i] = DOT3(V, U);
    }

    double l = min4(x[0], x[1], x[2], x[3]);
    double r = max4(x[0], x[1], x[2], x[3]);
    double b = min4(y[0], y[1], y[2], y[3]);
    double t = max4(y[0], y[1], y[2], y[3]);

    user_points[0][0] = C[0] + R[0] * l + U[0] * b;
    user_points[0][1] = C[1] + R[1] * l + U[1] * b;
    user_points[0][2] = C[2] + R[2] * l + U[2] * b;

    user_points[1][0] = C[0] + R[0] * r + U[0] * b;
    user_points[1][1] = C[1] + R[1] * r + U[1] * b;
    user_points[1][2] = C[2] + R[2] * r + U[2] * b;

    user_points[2][0] = C[0] + R[0] * l + U[0] * t;
    user_points[2][1] = C[1] + R[1] * l + U[1] * t;
    user_points[2][2] = C[2] + R[2] * l + U[2] * t;

    user_points[3][0] = C[0] + R[0] * r + U[0] * t;
    user_points[3][1] = C[1] + R[1] * r + U[1] * t;
    user_points[3][2] = C[2] + R[2] * r + U[2] * t;

    calc_user_planes(p);
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
    if (d)
    {
        if (m & KMOD_CTRL)
        {
            double P, T, R, p, y, r, s, F, A;

            s = ((m & KMOD_CAPS) || (m & KMOD_ALT)) ? 0.1 : 1.0;

            get_calibration(P, T, R, p, y, r, F, A);

            if (m & KMOD_SHIFT)
            {
                if      (k == SDLK_LEFT)     T += 1.0 * s;
                else if (k == SDLK_RIGHT)    T -= 1.0 * s;
                else if (k == SDLK_UP)       P += 1.0 * s;
                else if (k == SDLK_DOWN)     P -= 1.0 * s;
                else if (k == SDLK_PAGEUP)   R += 1.0 * s;
                else if (k == SDLK_PAGEDOWN) R -= 1.0 * s;
            }
            else
            {
                if      (k == SDLK_LEFT)     y += 1.0 * s;
                else if (k == SDLK_RIGHT)    y -= 1.0 * s;
                else if (k == SDLK_UP)       p += 1.0 * s;
                else if (k == SDLK_DOWN)     p -= 1.0 * s;
                else if (k == SDLK_PAGEUP)   r += 1.0 * s;
                else if (k == SDLK_PAGEDOWN) r -= 1.0 * s;
                else if (k == SDLK_HOME)     F += 1.0 * s;
                else if (k == SDLK_END)      F -= 1.0 * s;
                else if (k == SDLK_INSERT)   A += 0.1 * s;
                else if (k == SDLK_DELETE)   A -= 0.1 * s;
            }

            set_calibration(P, T, R, p, y, r, F, A);
            calc_calibrated();

            return true;
        }
    }
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

void app::frustum::pick(double *p, double *q, double x, double y) const
{
    // Return the pick vector for (x, y) in the current user space.

    double B[16], k = 1.0 - x - y;

    load_idt(B);

    B[ 8] = user_pos[0] - (user_points[2][0] * k +
                           user_points[3][0] * x +
                           user_points[0][0] * y);
    B[ 9] = user_pos[1] - (user_points[2][1] * k +
                           user_points[3][1] * x +
                           user_points[0][1] * y);
    B[10] = user_pos[2] - (user_points[2][2] * k +
                           user_points[3][2] * x +
                           user_points[0][2] * y);

    normalize(B + 8);
    crossprod(B + 0, B + 4, B + 8);
    normalize(B + 0);
    crossprod(B + 4, B + 8, B + 0);
    normalize(B + 4);

    get_quaternion(q, B);

    p[0] = user_pos[0];
    p[1] = user_pos[1];
    p[2] = user_pos[2];
}

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
