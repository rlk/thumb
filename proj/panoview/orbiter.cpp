//  Copyright (C) 2005-2011 Robert Kooima
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

#include <SDL_mouse.h>

#include <cmath>

#include <ogl-opengl.hpp>

#include <etc-socket.hpp>
#include <etc-math.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "scm/util3d/math3d.h"
#include "scm/scm-log.hpp"

#include "orbiter.hpp"

#define OLDINPUT 0

//------------------------------------------------------------------------------

static in_addr_t lookup(const std::string& hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname.c_str())))
    {
        memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);
        return  A.s_addr;
    }
    return INADDR_NONE;
}

//------------------------------------------------------------------------------

orbiter::orbiter(const std::string& exe,
                 const std::string& tag)
    : view_app(exe, tag)
{
    // Initialize all interaction state.

    speed_min   = ::conf->get_f("orbiter_speed_min",   0.0);
    speed_max   = ::conf->get_f("orbiter_speed_max",   0.5);
    minimum_agl = ::conf->get_f("orbiter_minimum_agl", 100.0);
    stick_timer = 0.0;

    // Initialize the reportage socket.

    report_addr.sin_family      = AF_INET;
    report_addr.sin_port        =  htons(::conf->get_i("orbiter_report_port"));
    report_addr.sin_addr.s_addr = lookup(::conf->get_s("orbiter_report_host"));

    if (report_addr.sin_addr.s_addr != INADDR_NONE)
        report_sock = socket(AF_INET, SOCK_DGRAM, 0);
    else
        report_sock = INVALID_SOCKET;

    // Preload data as requested.

    if (char *name = getenv("SCMINIT"))
    {
        load_file(name);

        if (sys && sys->get_step_count())
            move_to(0);
    }
}

orbiter::~orbiter()
{
    close(report_sock);
}

//------------------------------------------------------------------------------

void orbiter::report()
{
    // If a report destination has been configured...

    if (report_addr.sin_addr.s_addr != INADDR_NONE &&
        report_sock                 != INVALID_SOCKET)
    {
        // Compute the current longitude, latitude, and altitude.

        double p[3], alt = here.get_distance();

        here.get_position(p);

        double lon = atan2(p[0], p[2]) * 180.0 / M_PI;
        double lat =  asin(p[1])       * 180.0 / M_PI;

        // Encode these to an ASCII string.

        char buf[128];
        sprintf(buf, "%+12.8f %+13.8f %17.8f\n", lat, lon, alt);

        // And send the string to the configured host.

        sendto(report_sock, buf, strlen(buf) + 1, 0,
               (const sockaddr *) &report_addr, sizeof (sockaddr_in));
    }
}

//------------------------------------------------------------------------------

ogl::range orbiter::prep(int frusc, const app::frustum *const *frusv)
{
    report();

    view_app::prep(frusc, frusv);

    // Compute a horizon line based upon altitude and minimum terrain height.

    const double r =      get_current_ground();
    const double m =      get_minimum_ground();
    const double d = here.get_distance();

    double n = 0.5 *     (d     - r    );
    double f = 1.0 * sqrt(d * d - m * m);

    return ogl::range(n, f);
}

void orbiter::draw(int frusi, const app::frustum *frusp, int chani)
{
    double  l[3];
    GLfloat L[4];

    here.get_light(l);

    L[0] = GLfloat(l[0]);
    L[1] = GLfloat(l[1]);
    L[2] = GLfloat(l[2]);
    L[3] = 0.0f;

    glLoadIdentity();

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    view_app::draw(frusi, frusp, chani);
    view_app::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

void orbiter::load_file(const std::string& name)
{
    view_app::load_file(name);

    if (here.get_distance() == 0.0)
        here.set_distance(2.0 * get_minimum_ground());
}

// Return an altitude scalar.

double orbiter::get_speed() const
{
    const double d = here.get_distance();
    const double h =      get_current_ground();
    const double k = (d - h) / h;

    if (k > speed_max) return speed_max;
    if (k < speed_min) return speed_min;

    return k;
}

// Compute the length of the Archimedean spiral with polar equation r = a theta.

double arclen(double a, double theta)
{
    double d = sqrt(1 + theta * theta);
    return a * (theta * d + log(theta + d)) / 2.0;
}

// Calculate the length of the arc of length theta along an Archimedean spiral
// that begins at radius r0 and ends at radius r1.

double spiral(double r0, double r1, double theta)
{
    double dr = fabs(r1 - r0);

    if (theta > 0.0)
    {
        if (dr > 0.0)
        {
            double a = dr / theta;
            return fabs(arclen(a, r1 / a) - arclen(a, r0 / a));
        }
        return theta * r0;
    }
    return dr;
}

int orbiter::move_to(int i)
{
    // Construct a path from here to there.

    if (delta == 0)
    {
        if (0 <= i && i < sys->get_step_count())
        {
            // Set the location and destination.

            scm_step *src = &here;
            scm_step *dst = sys->get_step(i);

            // Determine the beginning and ending positions and altitudes.

            double p0[3];
            double p1[3];

            src->get_position(p0);
            dst->get_position(p1);

            double g0 = sys->get_current_ground(p0);
            double g1 = sys->get_current_ground(p1);

            double d0 = src->get_distance();
            double d1 = dst->get_distance();

            // Compute the ground trace length and orbit length.

            double a = acos(vdot(p0, p1));
            double lg = spiral(g0, g1, a);
            double lo = spiral(d0, d1, a);

            // Calculate a "hump" for a low orbit path.

            double aa = std::min(d0 - g0, d1 - g1);
            double dd = lg ? log10(lg / aa) * lg / 10 : 0;

            // Enqueue the path.

            sys->flush_queue();

            if (lo > 0)
                for (double t = 0.0; t < 1.0; )
                {
                    double p[3];
                    double dt = 0.01;
                    double q = 4 * t - 4 * t * t;

                    // Estimate the current velocity.

                    scm_step t0(src, dst, t);
                    scm_step t1(src, dst, t + dt);

                    t0.set_distance(t0.get_distance() + dd * q);
                    t1.set_distance(t1.get_distance() + dd * q);

                    // Queue this step.

                    if (t < 0.5)
                    {
                        t0.set_foreground(src->get_foreground());
                        t0.set_background(src->get_background());
                    }
                    else
                    {
                        t0.set_foreground(dst->get_foreground());
                        t0.set_background(dst->get_background());
                    }
                    sys->append_queue(new scm_step(t0));

                    // Move forward at a velocity appropriate for the altitude.

                    t0.get_position(p);

                    double g = sys->get_current_ground(p);

                    t += 2 * (t0.get_distance() - g) * dt * dt / (t1 - t0);
                }

            sys->append_queue(new scm_step(dst));

            // Trigger playback.

            now   = 0;
            delta = 1;
        }
    }
    return i;
}

int orbiter::fade_to(int i)
{
    // Construct a path from here to there.
#if 0
    if (delta == 0)
    {
        if (0 <= i && i < sys->get_step_count())
        {
            // Source and destination both remain fixed.

            path_src = here;
            path_dst = here;

            // Change the scene to that of the requested step.

            path_src.set_speed(1.0);
            path_dst.set_speed(1.0);
            path_dst.set_foreground(sys->get_step(i)->get_foreground());
            path_dst.set_background(sys->get_step(i)->get_background());

            // Queue these new steps and trigger playback.

            sys->flush_queue();
            sys->append_queue(&path_src);
            sys->append_queue(&path_dst);

            now   = 0;
            delta = 1;
        }
    }
#endif
    return i;
}

//------------------------------------------------------------------------------

// Compute the basis B of the default orientation for the given view V.

static void orbiting_basis(double *B, const double *V)
{
    load_idt(B);

    B[4] = V[12];
    B[5] = V[13];
    B[6] = V[14];

    normalize(B + 4);
    crossprod(B + 8, V + 0, B + 4);
    normalize(B + 8);
    crossprod(B + 0, B + 4, B + 8);
    normalize(B + 0);
}

// The navigate function modifies the current view matrix. In the case of a
// view_app, this entails updating the "here" step. In the case of an orbiter,
// this also entails re-orienting the navigation relative to the location on
// the planet, scaling the motion relative to the altitude, and enforcing
// the ground contact constraint.

void orbiter::navigate(const double *M)
{    
    const double *V = ::view->get_move_matrix();
    const double  k = 500000.0 * get_speed();
    const double  d = sqrt(DOT3(V + 12, V + 12));

    double X[16];
    double Z[16];
    double N[16];
    double R[16];
    double T[16];
    double B[16];
    double C[16];
    double O[16];
    double n[3];
    double m[3];

    load_mat(R, M);
    R[ 3] = 0.0;
    R[ 7] = 0.0;
    R[11] = 0.0;
    R[12] = 0.0;
    R[13] = 0.0;
    R[14] = 0.0;
    R[15] = 1.0;

    load_mat(O, V);
    O[ 3] = 0.0;
    O[ 7] = 0.0;
    O[11] = 0.0;
    O[12] = 0.0;
    O[13] = 0.0;
    O[14] = 0.0;
    O[15] = 1.0;

    orbiting_basis(B, V);
    load_inv      (C, B);

    // Calculate the motion vector in default space, via world space.

    mult_mat_vec3(n, O, M + 12);
    mult_mat_vec3(m, C, n);

    // Scale the motion vector to the current maximum speed.

    m[0] *= k;
    m[1] *= k;
    m[2] *= k;

    // Generate rotations to represent the motion.

    double za = DEG(atan2(-m[0], d));
    double xa = DEG(atan2( m[2], d));

    load_rot_mat(X, B[ 0], B[ 1], B[ 2], xa);
    load_rot_mat(Z, B[ 8], B[ 9], B[10], za);

    // Exploit a step object to apply these rotations correctly.

    scm_step S = here;

    S.set_matrix(::view->get_move_matrix());
    S.transform_orientation(X);
    S.transform_position(X);
    // S.transform_light(X);
    S.transform_orientation(Z);
    S.transform_position(Z);
    // S.transform_light(Z);

    // Apply the change in altitude, constrained to the terrain height.

    double p[3];

    S.get_position(p);
    S.set_distance(std::max(S.get_distance() + m[1],
                         sys->get_current_ground(p) + minimum_agl));

    // Finally apply the look transformation and store the result.

    S.get_matrix(T);
    mult_mat_mat(N, T, R);
    S.set_matrix(N);

    ::view->set_move_matrix(N);
    here = S;
}

void orbiter::get_up_vector(double *v)
{
    const double *I = ::view->get_move_inverse();
    v[0] = -I[12];
    v[1] = -I[13];
    v[2] = -I[14];
    normalize(v);
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        app::prog *P;

        P = new orbiter(argv[0], std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
        P->run();

        delete P;
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "Exception: %s\n", e.what());
    }
    return 0;
}

//------------------------------------------------------------------------------
