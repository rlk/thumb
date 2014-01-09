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

#include <etc-vector.hpp>
#include <app-default.hpp>
#include <ogl-opengl.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>

//-----------------------------------------------------------------------------

app::frustum::frustum() : n(1), f(100)
{
}

// Load the frustum onto the current OpenGL context.

void app::frustum::load_transform() const
{
    // Load the projection onto the OpenGL projection matrix.

    glMatrixMode(GL_PROJECTION);
    {
        glLoadMatrixd(transpose(get_transform()));
    }
    glMatrixMode(GL_MODELVIEW);

    // Clipping plane uniforms convey near and far distances.

    glPushMatrix();
    {
        glLoadIdentity();
        glClipPlane(GL_CLIP_PLANE0, vec4(0, 0, 1, n));
        glClipPlane(GL_CLIP_PLANE1, vec4(0, 0, 1, f));
    }
    glPopMatrix();
}

// Return a perspective projection matrix for this frustum.

mat4 app::frustum::get_transform() const
{
    return get_transform(n, f);
}

// Set the frustum view point.

void app::frustum::set_eye(const vec3& p)
{
    eye = p;
}

// Compute the world-space frustum bounding planes with the given view matrix.

void app::frustum::set_view(const mat4& V)
{
    n =   1.0;
    f = 100.0;
    cache_planes(get_transform() * V);
}

//-----------------------------------------------------------------------------

// Cache the display coordinate system basis in user space.

void app::frustum::cache_basis()
{
    vec3 x = normal(corner[1] - corner[0]);
    vec3 y = normal(corner[2] - corner[0]);
    vec3 z = normal(cross(x, y));

    basis = mat3(x, y, z);
}

// Calculate and store the bounding planes of the transformed projection.

void app::frustum::cache_planes(const mat4& A)
{
    const mat4 B = transpose(A);

    plane[0] = normal(B * vec4( 0,  0,  1,  1)); // N
    plane[1] = normal(B * vec4( 1,  0,  0,  1)); // L
    plane[2] = normal(B * vec4(-1,  0,  0,  1)); // R
    plane[3] = normal(B * vec4( 0,  1,  0,  1)); // B
    plane[4] = normal(B * vec4( 0, -1,  0,  1)); // T
    plane[5] = normal(B * vec4( 0,  0, -1,  1)); // F
}

// Calculate and store the corner vectors of the transformed frustum.

void app::frustum::cache_points(const mat4& A)
{
    const mat4 B = inverse(A);

    point[0] = B * vec3(-1, -1, -1); // BLN
    point[1] = B * vec3( 1, -1, -1); // BRN
    point[2] = B * vec3(-1,  1, -1); // TLN
    point[3] = B * vec3( 1,  1, -1); // TRN
    point[4] = B * vec3(-1, -1,  1); // BLF
    point[5] = B * vec3( 1, -1,  1); // BRF
    point[6] = B * vec3(-1,  1,  1); // TLF
    point[7] = B * vec3( 1,  1,  1); // TRF
}

//-----------------------------------------------------------------------------

// Find an eye-space position and orientation for the given screen coordinate.

bool app::frustum::pointer_to_3D(event *E, double s, double t) const
{
    // Find the eye-space line through the normalized view volume.

    const mat4 I = inverse(get_transform());

    vec3 a = I * vec3(2 * s - 1, 2 * t - 1, -1);
    vec3 b = I * vec3(2 * s - 1, 2 * t - 1,  1);

    // Find a basis oriented along the resulting vector.

    vec3 x = vec3(1, 0, 0);
    vec3 y = vec3(0, 1, 0);
    vec3 z = normal(a - b);

    x = normal(cross(y, z));
    y = normal(cross(z, x));

    quat q(mat3(x, y, z));

    // Store the pointer origin and orientation in the event.

    E->mk_point(0, a, q);

    return true;
}

// Find the 2D screen coordinate of the given eye-space 3D pointer event.

bool app::frustum::pointer_to_2D(event *E, double &s, double &t) const
{
    const mat4 A = get_transform();

    // Extract the pointer position and direction.

    vec3 p(E->data.point.p[0],
           E->data.point.p[1],
           E->data.point.p[2]);
    quat q(E->data.point.q[0],
           E->data.point.q[1],
           E->data.point.q[2],
           E->data.point.q[3]);
    vec3 v = -zvector(mat3(q));

    // Transform these into normalized device coordinates.

    vec3 a = project(A * vec4(p,     1));
    vec3 b = project(A * vec4(p + v, 1));
    vec3 d = b - a;

    // Determine where this line intersects the near plane.

    vec3 c = a + d * (a[2] - 1.0) / d[2];

    // Return true if the pointer falls within the view.

    if (-1 < c[0] && c[0] < 1 && -1 < c[1] && c[1] < 1)
    {
        s = (c[0] + 1) / 2;
        t = (c[1] + 1) / 2;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

/// Compute and return the "practical" parallel-split shadow map coefficient,
/// the average of the theoretically-optimal solution and the non-solution.

double app::frustum::get_split_c(int i, int m) const
{
    return (n * pow(f / n,   double(i) / double(m)) +
            n +    (f - n) * double(i) / double(m)) / 2;
}

/// Compute and return the linear fraction of a parallel-split coefficient.

double app::frustum::get_split_k(int i, int m) const
{
    const double c = get_split_c(i, m);
    return (c - n) / (f - n);
}

/// Compute and return the depth value of a parallel-split coefficient. This
/// is given in the same coordinate system as gl_FragCoord.z.

double app::frustum::get_split_z(int i, int m) const
{
    const double c = get_split_c(i, m);
    return (c - n) / (f - n) * (f / c);
}

/// Compute and return the axis-aligned bounding volume of a parallel-split

ogl::aabb app::frustum::get_split_bound(int i, int m) const
{
    const double k0 = get_split_k(i,     m);
    const double k1 = get_split_k(i + 1, m);

    ogl::aabb b;

    b.merge(mix(point[0], point[4], k0));
    b.merge(mix(point[0], point[4], k1));
    b.merge(mix(point[1], point[5], k0));
    b.merge(mix(point[1], point[5], k1));
    b.merge(mix(point[2], point[6], k0));
    b.merge(mix(point[2], point[6], k1));
    b.merge(mix(point[3], point[7], k0));
    b.merge(mix(point[3], point[7], k1));

    return b;
}

//-----------------------------------------------------------------------------

// Construct an orthogonal frustum covering bound b as seen from direction v.
// This is for use in generating shadow maps for directional light sources.

app::orthogonal_frustum::orthogonal_frustum(const ogl::aabb& b, const vec3 &v)
{
    vec3 x(1, 0, 0);
    vec3 y(0, 1, 0);
    vec3 z = normal(v);

    // Compute a basis for the light orientation. Beware of the vertical.

    if (fabs(z * y) < 1.0)
    {
        x = normal(cross(y, z));
        y = normal(cross(z, x));
    }
    else
    {
        y = normal(cross(z, x));
        x = normal(cross(y, z));
    }

    basis = mat4(mat3(x, y, z));

    // Determine the visible bound, axis-aligned in light space.

    ogl::aabb c(b, transpose(basis));

    vec3 p = c.min();
    vec3 q = c.max();

    // This gives the frustum parameters.

    f = -p[2];
    n = -q[2];

    corner[0] = vec3(p[0], p[1], n);
    corner[1] = vec3(q[0], p[1], n);
    corner[2] = vec3(p[0], q[1], n);
    corner[3] = vec3(q[0], q[1], n);

    cache_planes(get_transform());
}

// Compute near and far clipping distances to enclose the given bound.

void app::orthogonal_frustum::set_bound(const mat4& V, const ogl::aabb& bound)
{
    const vec4 p = vec4(plane[0][0], plane[0][1], plane[0][2], 0);

    if (bound.isvalid())
    {
        n = bound.min(p);
        f = bound.max(p);
    }
    cache_points(get_transform() * V);
}

mat4 app::orthogonal_frustum::get_transform(double N, double F) const
{
    double l = corner[0][0];
    double r = corner[1][0];
    double b = corner[0][1];
    double t = corner[2][1];
    return orthogonal(l, r, b, t, N, F) * transpose(basis) * translation(-eye);
}

mat4 app::orthogonal_frustum::get_transform() const
{
    return get_transform(n, f);
}

//-----------------------------------------------------------------------------

// Construct a perspective projection with the given aspect ratio and field of
// view in degrees. Position it at point p looking in direction v. This is for
// use in generating shadow maps for spot light sources.

app::perspective_frustum::perspective_frustum(const vec3& p,
                                              const vec3& v, double f, double a)
{
    vec3 x(1, 0, 0);
    vec3 y(0, 1, 0);
    vec3 z = normal(-v);

    // Compute a basis for the light orientation. Beware of the vertical.

    if (fabs(z * y) < 1.0)
    {
        x = normal(cross(y, z));
        y = normal(cross(z, x));
    }
    else
    {
        y = normal(cross(z, x));
        x = normal(cross(y, z));
    }

    basis = mat4(mat3(x, y, z));

    // Use it to calculate frustum corner positions.

    const double s = tan(to_radians(f / 2));

    corner[0] = p + v - x * s - y * s / a;
    corner[1] = p + v + x * s - y * s / a;
    corner[2] = p + v - x * s + y * s / a;
    corner[3] = p + v + x * s + y * s / a;

    eye = p;

    cache_planes(get_transform());
}

// Construct a simple frustum with the given projection.

app::perspective_frustum::perspective_frustum(const mat4& A)
{
    mat4 I = inverse(A);

    corner[0] = I * vec3(-1, -1, -1);
    corner[1] = I * vec3( 1, -1, -1);
    corner[2] = I * vec3(-1,  1, -1);
    corner[3] = I * vec3( 1,  1, -1);

    cache_basis();
}

// Construct a default perspective frustum.

app::perspective_frustum::perspective_frustum()
{
    double x = tan(to_radians(DEFAULT_HORZ_FOV / 2.0));
    double y = x * double(DEFAULT_PIXEL_HEIGHT)
                 / double(DEFAULT_PIXEL_WIDTH);

    corner[0] = vec3(-x, -y, -1);
    corner[1] = vec3( x, -y, -1);
    corner[2] = vec3(-x,  y, -1);
    corner[3] = vec3( x,  y, -1);
}

// Compute near and far clipping distances to enclose the given bound.

void app::perspective_frustum::set_bound(const mat4& V, const ogl::aabb& bound)
{
    if (bound.isvalid())
    {
        const mat4 M = transpose(basis) * translation(-eye) * V;

        ogl::aabb bb(bound, M);

        n = bb.min(vec4(0.0, 0.0, -1.0, 0.0));
        f = bb.max(vec4(0.0, 0.0, -1.0, 0.0));

        if (n < 0.1)
            n = 0.1;
    }
    cache_points(get_transform() * V);
}

// Return a perspective projection matrix for this frustum.

mat4 app::perspective_frustum::get_transform(double N, double F) const
{
    mat4 A = transpose(mat3(normal(corner[0] - eye),
                            normal(corner[1] - eye),
                            normal(corner[2] - eye))) * basis;

    double l = -N * A[0][0] / A[0][2];
    double r = -N * A[1][0] / A[1][2];
    double b = -N * A[0][1] / A[0][2];
    double t = -N * A[2][1] / A[2][2];

    return perspective(l, r, b, t, N, F) * transpose(basis) * translation(-eye);
}

mat4 app::perspective_frustum::get_transform() const
{
    return get_transform(n, f);
}

//-----------------------------------------------------------------------------

// Construct a calibrated frustum using the given serialization node.

app::calibrated_frustum::calibrated_frustum(app::node n) : node(n)
{
    apply_calibration();
}

// A default calibrated frustum is just a default perspective frustum.

app::calibrated_frustum::calibrated_frustum()
{
}

// Convert the current calibration to a transformation matrix.

mat4 app::calibrated_frustum::calibration_matrix(calibration& C)
{
    return yrotation  (to_radians(C.T))    // Position theta
         * xrotation  (to_radians(C.P))    // Position phi
         * translation(vec3(0, 0, C.R))    // Position rho
         * yrotation  (to_radians(C.y))    // Rotation yaw
         * xrotation  (to_radians(C.p))    // Rotation pitch
         * zrotation  (to_radians(C.r));   // Rotation roll
}

// Extract the frustum definition from the serialization node.

void app::calibrated_frustum::apply_calibration()
{
    int  b = 0;
    vec3 c[4];
    mat4 T;

    double hfov = DEFAULT_HORZ_FOV;
    double vfov = DEFAULT_VERT_FOV;

    if (node)
    {
        // Extract the screen corners.

        for (app::node n = node.find("corner"); n; n = node.next(n, "corner"))
        {
            const std::string name = n.get_s("name");
            int i = 0;

            // Determine which corner is being specified.

            if (!name.empty())
            {
                if      (name == "BL") { i = 0; b |= 1; }
                else if (name == "BR") { i = 1; b |= 2; }
                else if (name == "TL") { i = 2; b |= 4; }
                else if (name == "TR") { i = 3; b |= 8; }
            }

            // Extract the position.

            const std::string unit = n.get_s("unit");

            double scale = scale_to_meters(unit.empty() ? "ft" : unit);

            c[i][0] = n.get_f("x") * scale;
            c[i][1] = n.get_f("y") * scale;
            c[i][2] = n.get_f("z") * scale;
        }

        // Extract fields-of-view.

        if (app::node n = node.find("perspective"))
        {
            hfov = n.get_f("hfov", DEFAULT_HORZ_FOV);
            vfov = n.get_f("vfov", DEFAULT_VERT_FOV);
        }
    }

    // Compute any one unspecified screen corner.

    if      (b == 14) c[0] = c[2] + c[1] - c[3];
    else if (b == 13) c[1] = c[0] + c[3] - c[2];
    else if (b == 11) c[2] = c[3] + c[0] - c[1];
    else if (b ==  7) c[3] = c[1] + c[2] - c[0];

    // ... or compute all four corners.

    else
    {
        const double x = tan(to_radians(hfov * 0.5));
        const double y = tan(to_radians(vfov * 0.5));

        c[0] = vec3(-x, -y, -1);
        c[1] = vec3(+x, -y, -1);
        c[2] = vec3(-x, +y, -1);
        c[3] = vec3(+x, +y, -1);
    }

    // Apply the calibration transform to the configured frustum corners.

    calibration C;

    load_calibration(C);

    mat4 M = calibration_matrix(C);

    corner[0] = M * c[0];
    corner[1] = M * c[1];
    corner[2] = M * c[2];
    corner[3] = M * c[3];

    cache_basis();
}

// Extract the calibration from the serialization node.

void app::calibrated_frustum::load_calibration(calibration& C)
{
    // Assign defaults for any undefined parameters.

    C.P = 0.0;               // Position phi
    C.T = 0.0;               // Position theta
    C.R = 0.0;               // Position rho
    C.p = 0.0;               // Rotation pitch
    C.y = 0.0;               // Rotation yaw
    C.r = 0.0;               // Rotation roll
    C.H = DEFAULT_HORZ_FOV;  // Horizontal field of view
    C.V = DEFAULT_VERT_FOV;  // Vertical   field of view

    if (node)
    {
        if (app::node n = node.find("position"))
        {
            C.P = n.get_f("p", C.P);
            C.T = n.get_f("t", C.T);
            C.R = n.get_f("r", C.R);
        }
        if (app::node n = node.find("rotation"))
        {
            C.p = n.get_f("p", C.p);
            C.y = n.get_f("y", C.y);
            C.r = n.get_f("r", C.r);
        }
        if (app::node n = node.find("perspective"))
        {
            C.H = n.get_f("hfov", C.H);
            C.V = n.get_f("vfov", C.V);
        }
    }
}

// Update the calibration in the serialization node.

void app::calibrated_frustum::save_calibration(calibration& C)
{
    if (node)
    {
        if (app::node n = node.find("position"))
        {
            n.set_f("p", C.P);
            n.set_f("t", C.T);
            n.set_f("r", C.R);
        }
        if (app::node n = node.find("rotation"))
        {
            n.set_f("p", C.p);
            n.set_f("y", C.y);
            n.set_f("r", C.r);
        }
        if (app::node n = node.find("perspective"))
        {
            n.set_f("hfov", C.H);
            n.set_f("vfov", C.V);
        }
    }
}

bool app::calibrated_frustum::process_event(app::event *E)
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
                if      (k == SDL_SCANCODE_LEFT)     { dT =  d; b = true; }
                else if (k == SDL_SCANCODE_RIGHT)    { dT = -d; b = true; }
                else if (k == SDL_SCANCODE_UP)       { dP =  d; b = true; }
                else if (k == SDL_SCANCODE_DOWN)     { dP = -d; b = true; }
                else if (k == SDL_SCANCODE_PAGEUP)   { dR =  d; b = true; }
                else if (k == SDL_SCANCODE_PAGEDOWN) { dR = -d; b = true; }
            }
            else
            {
                if      (k == SDL_SCANCODE_LEFT)     { dy = -d; b = true; }
                else if (k == SDL_SCANCODE_RIGHT)    { dy =  d; b = true; }
                else if (k == SDL_SCANCODE_UP)       { dp = -d; b = true; }
                else if (k == SDL_SCANCODE_DOWN)     { dp =  d; b = true; }
                else if (k == SDL_SCANCODE_PAGEUP)   { dr =  d; b = true; }
                else if (k == SDL_SCANCODE_PAGEDOWN) { dr = -d; b = true; }
                else if (k == SDL_SCANCODE_INSERT)   { dH =  d; b = true; }
                else if (k == SDL_SCANCODE_DELETE)   { dH = -d; b = true; }
                else if (k == SDL_SCANCODE_HOME)     { dV =  d; b = true; }
                else if (k == SDL_SCANCODE_END)      { dV = -d; b = true; }
            }

            // If changes occurred, apply them to the calibration.

            if (b)
            {
                calibration C;

                load_calibration(C);
                C.P += dP;
                C.T += dT;
                C.R += dR;
                C.p += dp;
                C.y += dy;
                C.r += dr;
                C.H += dH;
                C.V += dV;
                save_calibration(C);

                apply_calibration();
                return true;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
