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

#include <cmath>
#include <cstring>

#include <SDL.h>
#include <SDL_keyboard.h>

#include <etc-math.hpp>
#include <app-default.hpp>
#include <ogl-opengl.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>

//-----------------------------------------------------------------------------

/// Constructor
///
/// This is the general-use frustum constructor. It receives an XML DOM node
/// for configuration serialization. The width and height parameters give the
/// resolution of a render target, if any.
///
/// By default, the frustum apex is placed at the origin (0, 0, 0), the view
/// transform is initialized to the identity, and the near and far distances
/// are set to 0.5 and 100.
///
/// \param node XML DOM node
/// \param w    Render target pixel width
/// \param h    Render target pixel height
///
app::frustum::frustum(app::node node, int w, int h) :
    node(node),
    pixel_w(w),
    pixel_h(h),
    user_angle(0),
    view_count(0)
{
    user_pos[0] = 0.0;
    user_pos[1] = 0.0;
    user_pos[2] = 0.0;

    disp_pos[0] = 0.0;
    disp_pos[1] = 0.0;
    disp_pos[2] = 0.0;

    // Load the configuration and perform a calibration.

    calc_calibrated();

    // Load sufficient defaults to allow immediate basic usage.

    double M[16];

    load_idt(M);

    set_viewpoint(user_pos);
    set_transform(M);
    set_distances(0.5, 100.0);
}

/// Copy constuctor
///
/// This constructor copies the state of the given frustum object, including
/// any internally cached values, producing a functionally equivalent
/// frustum. However it does NOT copy the given frustum's XML DOM node
/// reference. This prevents a copy from overwriting the original's
/// serialization, but precludes all serialization by frustum copies.
///
/// \param that Frustum to be copied
///
app::frustum::frustum(const frustum& that) :
    node(0),
    pixel_w(that.pixel_w),
    pixel_h(that.pixel_h),
    user_angle(that.user_angle),
    view_count(0)
{
    memcpy(user_pos,    that.user_pos,        3 * sizeof (double));
    memcpy(disp_pos,    that.disp_pos,        3 * sizeof (double));
    memcpy(user_points, that.user_points, 4 * 3 * sizeof (double));
    memcpy(view_points, that.view_points, 8 * 3 * sizeof (double));
    memcpy(view_planes, that.view_planes, 6 * 4 * sizeof (double));
    memcpy(user_basis,  that.user_basis,     16 * sizeof (double));
    memcpy(P,           that.P,              16 * sizeof (double));
}

//-----------------------------------------------------------------------------

void app::frustum::set_projection(const double *M)
{
    const double u0[3] = { -1.0, -1.0, -1.0 };
    const double u1[3] = {  1.0, -1.0, -1.0 };
    const double u2[3] = { -1.0,  1.0, -1.0 };
    const double u3[3] = {  1.0,  1.0, -1.0 };

    double I[16];

    load_inv(I, M);

    mult_mat_vec3(user_points[0], I, u0);
    mult_mat_vec3(user_points[1], I, u1);
    mult_mat_vec3(user_points[2], I, u2);
    mult_mat_vec3(user_points[3], I, u3);

    calc_basis();
}

/// Set the near and far clipping distances.
///
/// The five corners defining the off-axis pyramid of a view frustum define a
/// volume that is effectively infinite. However, perspective projection
/// requires a bounded volume, capped at the minimum and maximum visible
/// distance. This function sets these distances, and must be called prior to
/// the use of the frustum's perspective projection matrix. In practice, the
/// near and far distances vary with the content of the scene, and are
/// recomputed each frame by the app::host.
///
/// \param n Near plane distance
/// \param f  Far plane distance
///
void app::frustum::set_distances(double n, double f)
{
    double u[4][3];
    double v[4][3];

    // Cache the near and far clipping plane distances.

    n_dist = n;
    f_dist = f;

    // Compute the world-space frustum corner vectors.

    crossprod(v[0], view_planes[3], view_planes[1]);
    crossprod(v[1], view_planes[2], view_planes[3]);
    crossprod(v[2], view_planes[1], view_planes[4]);
    crossprod(v[3], view_planes[4], view_planes[2]);

    // For each view frustum corner...

    for (int i = 0; i < 4; ++i)
    {
        normalize(v[i]);

        const double k = DOT3(v[i], view_planes[0]);

        // Cache the world-space near position.

        view_points[i    ][0] = view_pos[0] + v[i][0] * n / k;
        view_points[i    ][1] = view_pos[1] + v[i][1] * n / k;
        view_points[i    ][2] = view_pos[2] + v[i][2] * n / k;

        // Cache the world-space far position.

        view_points[i + 4][0] = view_pos[0] + v[i][0] * f / k;
        view_points[i + 4][1] = view_pos[1] + v[i][1] * f / k;
        view_points[i + 4][2] = view_pos[2] + v[i][2] * f / k;

        // Compute the user-space frustum corner vector.

        u[i][0] = user_points[i][0] - user_pos[0];
        u[i][1] = user_points[i][1] - user_pos[1];
        u[i][2] = user_points[i][2] - user_pos[2];

        normalize(u[i]);
    }

    // Generate the off-axis projection.

    const double x0 = DOT3(user_basis + 0, u[0]);
    const double x1 = DOT3(user_basis + 0, u[1]);

    const double y0 = DOT3(user_basis + 4, u[0]);
    const double y2 = DOT3(user_basis + 4, u[2]);

    const double z0 = DOT3(user_basis + 8, u[0]);
    const double z1 = DOT3(user_basis + 8, u[1]);
    const double z2 = DOT3(user_basis + 8, u[2]);

    const double l = -n * x0 / z0;
    const double r = -n * x1 / z1;
    const double b = -n * y0 / z0;
    const double t = -n * y2 / z2;

    load_persp(P, l, r, b, t, n, f);

    // Orient the projection and move the apex to the origin.

    double A[16];

    load_xps(A, user_basis);

    mult_mat_mat(P, P, A);
    Rmul_xlt_inv(P, user_pos[0],
                    user_pos[1],
                    user_pos[2]);
}

/// Set the position of the apex of the view frustum.
///
/// The view frustum apex remains fixed for most applications, but can vary in
/// response to a user's movements within a user-tracked viewer-centric virtual
/// reality environment. Visibility testing must respond properly, and this
/// function makes that possible. The given position is usually the output of
/// the head sensor of a 3D tracking system and is given in the same user-space
/// coordinate system as the frustum corners.
///
/// \param p 3D position vector
///
void app::frustum::set_viewpoint(const double *p)
{
    // Cache the user position.

    user_pos[0] = p[0];
    user_pos[1] = p[1];
    user_pos[2] = p[2];

    // Compute the vector from the screen center to the viewer.

    double v[3];

    v[0] = user_pos[0] - (user_points[0][0] + user_points[3][0]) * 0.5;
    v[1] = user_pos[1] - (user_points[0][1] + user_points[3][1]) * 0.5;
    v[2] = user_pos[2] - (user_points[0][2] + user_points[3][2]) * 0.5;

    mult_xps_vec3(disp_pos, user_basis, v);

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
}

/// Set the 3D view transformation.
///
/// \param M 4x4 transformation matrix.
///
void app::frustum::set_transform(const double *M)
{
    // Cache the world-space view position.

    mult_mat_vec3(view_pos, M, user_pos);

    // Cache the world-space display corners.

    mult_mat_vec3(view_points[0], M, user_points[0]);
    mult_mat_vec3(view_points[1], M, user_points[1]);
    mult_mat_vec3(view_points[2], M, user_points[2]);
    mult_mat_vec3(view_points[3], M, user_points[3]);

    // Cache the world-space view frustum bounding planes.

    set_plane(view_planes[0], view_points[1], view_points[0], view_points[2]);
    set_plane(view_planes[1], view_pos, view_points[0], view_points[2]); // L
    set_plane(view_planes[2], view_pos, view_points[3], view_points[1]); // R
    set_plane(view_planes[3], view_pos, view_points[1], view_points[0]); // B
    set_plane(view_planes[4], view_pos, view_points[2], view_points[3]); // T

    // Force the near clipping plane to pass through the view point.

    view_planes[0][3] = -DOT3(view_pos, view_planes[0]);

    view_count = 5;
}

void app::frustum::set_horizon(double r)
{
    // Use the view position and given radius to compute the horizon plane.

    view_planes[5][0] = view_pos[0];
    view_planes[5][1] = view_pos[1];
    view_planes[5][2] = view_pos[2];

    normalize(view_planes[5]);

    view_planes[5][3] = -r * r / sqrt(DOT3(view_pos, view_pos));

    view_count = 6;
}

//-----------------------------------------------------------------------------

/// Initialize this frustum to cover a given volume of the scene.
///
/// This function implements the core of the parallel-split shadow mapping
/// algorithm. It computes all values needed to render a shadow map for the
/// subset of the scene falling within a given volume.
///
/// The primary input is a vector of view frusta which are assumed to be
/// spacially related, either as two elements of a stereo pair, or as adjacent
/// views of a tiled display, or both. A pair of values give near and far
/// planes selecting a subset of these view volumes. Finally a 3D vector gives
/// the position of a light source.
///
/// The objective of the function is to compute a view frustum covering the
/// selected union of given frusta, and to return matrices that transform to
/// and from the coordinate system of the light source.
///
/// \param[in]  frusc Number of view frusta
/// \param[in]  frusv Vector of view frusta
/// \param[in]  c0    View frustum near coefficient
/// \param[in]  c1    View frustum far  coefficient
/// \param[in]  L     3D Light source position
/// \param[out] M     4x4 Light source coordinate system transform
/// \param[out] I     4x4 Light source coordinate system inverse
///
void app::frustum::set_volume(int frusc, const app::frustum *const *frusv,
                              double c0, double c1, const double *L,
                              double *M, double *I)
{
    // Compute a light frustum encompassing the given array of frustums, over
    // the view-space range C0 to C1, as seen by the light at position L.

    double l =  std::numeric_limits<double>::max();
    double r = -std::numeric_limits<double>::max();
    double b =  std::numeric_limits<double>::max();
    double t = -std::numeric_limits<double>::max();
    double n =  std::numeric_limits<double>::max();
    double f = -std::numeric_limits<double>::max();

    // Begin by computing the center point of all C0 to C1 subsets.

    double C[3] = { 0.0, 0.0, 0.0 }, cc = 0.5 * (c0 + c1), dd = 1.0 - cc;

    for (int frusi = 0; frusi < frusc; ++frusi)
    {
        const frustum *f = frusv[frusi];

        C[0] += (cc * f->view_points[0][0] + dd * f->view_points[4][0] +
                 cc * f->view_points[3][0] + dd * f->view_points[7][0]) * 0.5;
        C[1] += (cc * f->view_points[0][1] + dd * f->view_points[4][1] +
                 cc * f->view_points[3][1] + dd * f->view_points[7][1]) * 0.5;
        C[2] += (cc * f->view_points[0][2] + dd * f->view_points[4][2] +
                 cc * f->view_points[3][2] + dd * f->view_points[7][2]) * 0.5;
    }

    C[0] /= frusc;
    C[1] /= frusc;
    C[2] /= frusc;

    // Compute the transform and inverse for the light's coordinate system...

    load_idt(M);

    // The Z axis is the vector from the light center to the light position.

    M[ 8] = L[0] - C[0];
    M[ 9] = L[1] - C[1];
    M[10] = L[2] - C[2];

    normalize(M + 8);

    // The Y axis is "up".

    if (L[1] > sqrt(L[0] * L[0] + L[2] * L[2]))
    {
        M[4] =  0.0;
        M[5] =  1.0;
        M[6] =  0.0;
    }
    else
    {
        M[4] =  0.0;
        M[5] =  0.0;
        M[6] = -1.0;
    }

    // The X axis is the cross product of these.

    crossprod(M + 0, M + 4, M + 8);
    normalize(M + 0);

    // Be sure the Y axis is orthogonal...

    crossprod(M + 4, M + 8, M + 0);
    normalize(M + 4);

    // The inverse basis is the transpose.

    load_xps(I, M);

    // Apply the lightsourse position transform to the matrix...

    M[12] = L[0];
    M[13] = L[1];
    M[14] = L[2];

    // ... and to the inverse.

    I[12] = -DOT3(L, M + 0);
    I[13] = -DOT3(L, M + 4);
    I[14] = -DOT3(L, M + 8);

    // Now transform each frustum into this space and find the extent union.

    for (int frusi = 0; frusi < frusc; ++frusi)
    {
        const frustum *F = frusv[frusi];
        const double  d0 = 1.0 - c0;
        const double  d1 = 1.0 - c1;

        for (int i = 0, j = 4; i < 4; ++i, ++j)
        {
            double p0[3], p1[3];
            double q0[3], q1[3];

            // Compute the corners of the C0 to C1 subset of this frustum.

            p0[0] = c0 * F->view_points[j][0] + d0 * F->view_points[i][0];
            p0[1] = c0 * F->view_points[j][1] + d0 * F->view_points[i][1];
            p0[2] = c0 * F->view_points[j][2] + d0 * F->view_points[i][2];

            p1[0] = c1 * F->view_points[j][0] + d1 * F->view_points[i][0];
            p1[1] = c1 * F->view_points[j][1] + d1 * F->view_points[i][1];
            p1[2] = c1 * F->view_points[j][2] + d1 * F->view_points[i][2];

            // Transform the corners of this frustum into light space.

            mult_mat_vec3(q0, I, p0);
            mult_mat_vec3(q1, I, p1);

            // Project these corners onto a common plane at unit distance.

            q0[0] /= -q0[2]; q0[1] /= -q0[2];
            q1[0] /= -q1[2]; q1[1] /= -q1[2];

            // Find the frustum extrema.

            l = std::min(l,  q0[0]);
            l = std::min(l,  q1[0]);
            r = std::max(r,  q0[0]);
            r = std::max(r,  q1[0]);
            b = std::min(b,  q0[1]);
            b = std::min(b,  q1[1]);
            t = std::max(t,  q0[1]);
            t = std::max(t,  q1[1]);
            n = std::min(n, -q0[2]);
            n = std::min(n, -q1[2]);
            f = std::max(f, -q0[2]);
            f = std::max(f, -q1[2]);
        }
    }

    n = std::max(n, 1.0);

    // Set up the frustum.

    load_idt(user_basis);

    user_points[0][0] =  l;
    user_points[0][1] =  b;
    user_points[0][2] = -1;

    user_points[1][0] =  r;
    user_points[1][1] =  b;
    user_points[1][2] = -1;

    user_points[2][0] =  l;
    user_points[2][1] =  t;
    user_points[2][2] = -1;

    user_points[3][0] =  r;
    user_points[3][1] =  t;
    user_points[3][2] = -1;

    double O[3] = { 0, 0, 0 };

    set_viewpoint(O);
    set_transform(M);

    // This frustum is now ready for view culling.
}

//-----------------------------------------------------------------------------

/// Return the position of the apex of the view frustum in user coordinates.
///
const double *app::frustum::get_user_pos() const
{
    return user_pos;
}

/// Return the position of the apex of the view frustum in world coordinates.
///
const double *app::frustum::get_view_pos() const
{
    return view_pos;
}

/// Return the position of the apex of the view frustum in display coordinates.
///
const double *app::frustum::get_disp_pos() const
{
    return disp_pos;
}

/// Return the 4x4 perspective projection matrix in OpenGL form.
/// \warning If app::frustum::set_viewpoint has been called then
/// app::frustum::set_distances must also be called before the
/// perspective projection may be retrieved.
///
const double *app::frustum::get_P() const
{
    return P;
}

//-----------------------------------------------------------------------------

/// Return the user-space width of the base of the frustum in meters.
///
double app::frustum::get_w() const
{
    double d[3];

    d[0] = user_points[1][0] - user_points[0][0];
    d[1] = user_points[1][1] - user_points[0][1];
    d[2] = user_points[1][2] - user_points[0][2];

    return sqrt(DOT3(d, d));
}

/// Return the user-space height of the base of the frustum in meters.
///
double app::frustum::get_h() const
{
    double d[3];

    d[0] = user_points[2][0] - user_points[0][0];
    d[1] = user_points[2][1] - user_points[0][1];
    d[2] = user_points[2][2] - user_points[0][2];

    return sqrt(DOT3(d, d));
}

/// Return the width of the base of the frustum in pixels.
///
int app::frustum::get_pixel_w() const
{
    return pixel_w;
}

/// Return the height of the base of the frustum in pixels.
///
int app::frustum::get_pixel_h() const
{
    return pixel_h;
}

#if 0
/// Estimate and return the number of pixels in the given solid angle.
///
/// Given a measurement of the solid angle subtended by an object (or its
/// bounding volume) and the resolution of the destination render buffer we can
/// closely estimate the total number of pixels generated while rendering this
/// object. This value may be used to tune texture quality or levels of detail.
///
/// \param a Solid angle in steradians
///
double app::frustum::pixels(double a) const
{
    // Estimate and return the number of pixels in the given solid angle.

    return pixel_w * pixel_h * a / user_angle;
}
#endif
//-----------------------------------------------------------------------------

/// Compute and return the parallel-split shadow map coefficient.
/// \param k = i / n selects split i out of n
///
double app::frustum::get_split_coeff(double k) const
{
    return (n_dist * pow(f_dist / n_dist,   k) +
            n_dist +    (f_dist - n_dist) * k) * 0.5;
}

/// Compute and return the linear fraction of a parallel-split coefficient.
/// \param c app::frustum::get_split_coeff value
///
double app::frustum::get_split_fract(double c) const
{
    return                (c - n_dist) / (f_dist - n_dist);
}

/// Compute and return the depth value of a parallel-split coefficient.
/// \param c app::frustum::get_split_coeff value
///
double app::frustum::get_split_depth(double c) const
{
    return (f_dist / c) * (c - n_dist) / (f_dist - n_dist);
}

//-----------------------------------------------------------------------------
#if 0
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

/// Test the visibility of a planetary surface shell.
///
/// This function tests the visibility of a bounding volume of a triangular
/// patch of spherical terrain. Consider a triangle inscribed on a planet, with
/// its three sides following geodesics on the sphere. This triangle is defined
/// by three vectors normal to the surface of the sphere. Let r0 be the minimum
/// terrain height within this triangle and r1 be the maximum. The volume
/// bounded by these five values takes the form of a curved triangular shell
/// which fully and tightly encloses all terrain within it.
///
/// \param n0 Normalized shell corner vector 0
/// \param n1 Normalized shell corner vector 1
/// \param n2 Normalized shell corner vector 2
/// \param r0 Shell inner radius
/// \param r1 Shell outer radius
/// \return positive if fully visible,
/// \return zero if partly visible,
/// \return negative if not visible.
///
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
#endif
//-----------------------------------------------------------------------------
#if 0
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
#endif
//-----------------------------------------------------------------------------

bool app::frustum::pointer_to_2D(event *E, int& x, int& y) const
{
    const double *p = E->data.point.p;
    const double *q = E->data.point.q;
    double M[16], v[3], n[4];

    // Determine the pointer vector from the quaternion.

    quat_to_mat(M, q);

    v[0] = -M[ 8];
    v[1] = -M[ 9];
    v[2] = -M[10];

    // Determine where the pointer intersects with the image plane.

    set_plane(n, user_points[0], user_points[1], user_points[2]);

    double t = -(DOT3(p, n) + n[3]) / DOT3(v, n);

    double P[3];
    double X[3];
    double Y[3];

    P[0] =   p[0] + t * v[0] - user_points[0][0];
    P[1] =   p[1] + t * v[1] - user_points[0][1];
    P[2] =   p[2] + t * v[2] - user_points[0][2];
    X[0] = user_points[1][0] - user_points[0][0];
    X[1] = user_points[1][1] - user_points[0][1];
    X[2] = user_points[1][2] - user_points[0][2];
    Y[0] = user_points[2][0] - user_points[0][0];
    Y[1] = user_points[2][1] - user_points[0][1];
    Y[2] = user_points[2][2] - user_points[0][2];

    double xx = DOT3(P, X) / DOT3(X, X);
    double yy = DOT3(P, Y) / DOT3(Y, Y);

    // If the pointer falls within the frustum, return the nearest pixel.

    if (0.0 <= xx && xx <= 1.0 && 0.0 <= yy && yy <= 1.0)
    {
        x = nearest_int(pixel_w * xx);
        y = nearest_int(pixel_h * yy);
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

    double q[4];

    normalize(B + 8);
    crossprod(B + 0, B + 4, B + 8);
    normalize(B + 0);
    crossprod(B + 4, B + 8, B + 0);
    normalize(B + 4);

    mat_to_quat(q, B);

    // Store the pointer origin and direction in the event.

    E->mk_point(0, user_pos, q);

    return true;
}

bool app::frustum::process_event(app::event *E)
{
    if (E->get_type() == E_KEY && E->data.key.d)
    {
        const int k = E->data.key.k;
        const int m = E->data.key.m;

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
    // Load the projection to the OpenGL projection matrix.

    glMatrixMode(GL_PROJECTION);
    {
        glLoadMatrixd(P);
    }
    glMatrixMode(GL_MODELVIEW);
}

void app::frustum::overlay() const
{
    // Produce a unit-to-pixel transformation for 2D overlay.

    glMatrixMode(GL_MODELVIEW);
    {
        glLoadIdentity();

        glTranslated(user_points[0][0],
                     user_points[0][1],
                     user_points[0][2]);

        glMultMatrixd(user_basis);

        glScaled(get_w() / pixel_w,
                 get_h() / pixel_h, 1.0);
    }
}

//-----------------------------------------------------------------------------

void app::frustum::get_calibration(double& P, double& T, double& R,
                                   double& p, double& y, double& r,
                                   double& H, double& V)
{
    // Assign defaults for any undefined parameters.

    P = 0.0;               // Position phi
    T = 0.0;               // Position theta
    R = 0.0;               // Position rho
    p = 0.0;               // Rotation pitch
    y = 0.0;               // Rotation yaw
    r = 0.0;               // Rotation roll
    H = DEFAULT_HORZ_FOV;  // Horizontal field of view
    V = DEFAULT_VERT_FOV;  // Vertical   field of view

    // Extract the calibration from the serialization node.

    if (node)
    {
        if (app::node n = node.find("position"))
        {
            P = n.get_f("p", P);
            T = n.get_f("t", T);
            R = n.get_f("r", R);
        }
        if (app::node n = node.find("rotation"))
        {
            p = n.get_f("p", p);
            y = n.get_f("y", y);
            r = n.get_f("r", r);
        }
        if (app::node n = node.find("perspective"))
        {
            H = n.get_f("hfov", H);
            V = n.get_f("vfov", V);
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
        if (app::node n = node.find("position"))
        {
            n.set_f("p", P);
            n.set_f("t", T);
            n.set_f("r", R);
        }
        if (app::node n = node.find("rotation"))
        {
            n.set_f("p", p);
            n.set_f("y", y);
            n.set_f("r", r);
        }
        if (app::node n = node.find("perspective"))
        {
            n.set_f("hfov", H);
            n.set_f("vfov", V);
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

void app::frustum::calc_basis()
{
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

void app::frustum::calc_calibrated()
{
    // Extract the frustum definition from the serialization node.

    bool b[4] = { false, false, false, false };

    double hfov = DEFAULT_HORZ_FOV;
    double vfov = DEFAULT_VERT_FOV;

    double T[16], c[4][3];

    if (node)
    {
        // Extract the screen corners.

        for (app::node n = node.find("corner"); n; n = node.next(n, "corner"))
        {
            const std::string name = n.get_s("name");
            double *v = 0;

            // Determine which corner is being specified.

            if (!name.empty())
            {
                if      (name == "BL") { v = c[0]; b[0] = true; }
                else if (name == "BR") { v = c[1]; b[1] = true; }
                else if (name == "TL") { v = c[2]; b[2] = true; }
                else if (name == "TR") { v = c[3]; b[3] = true; }
            }

            if (v)
            {
                const std::string unit = n.get_s("unit");

                double scale = scale_to_meters(unit.empty() ? "ft" : unit);

                // Extract the position.

                v[0] = n.get_f("x") * scale;
                v[1] = n.get_f("y") * scale;
                v[2] = n.get_f("z") * scale;
            }
        }

        // Extract fields-of-view.

        if (app::node n = node.find("perspective"))
        {
            hfov = n.get_f("hfov");
            vfov = n.get_f("vfov");
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

    calc_basis();
}

//-----------------------------------------------------------------------------
