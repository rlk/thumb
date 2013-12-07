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
#include <etc-vector.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "scm/util3d/math3d.h"
#include "scm/scm-log.hpp"

#include "orbiter.hpp"

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
    // Set the light position.

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

//------------------------------------------------------------------------------

quat orbiter::get_local() const
{
    const vec3 p(::view->get_position());
    const mat3 R(::view->get_orientation());

    vec3 x = normal(xvector(R));
    vec3 y = normal(p);
    vec3 z = normal(cross(x, y));

    return quat(mat3(x, y, z));
}

quat orbiter::get_orientation() const
{
    return inverse(get_local()) * ::view->get_orientation();
}

void orbiter::set_orientation(const quat &q)
{
    quat r = normal(get_local() * q);
    here.set_orientation(r);
    ::view->set_orientation(r);
}

void orbiter::offset_position(const vec3 &d)
{
    // Set the current step using the current camera configuration.

    step_from_view(here);

    // Apply the motion vector as rotation of the step.

    const double k = 500000.0 * get_speed();
    const double r = here.get_distance();
    const mat3   B(get_local());

    mat4 zM = mat4(mat3(quat(zvector(B), atan2(-d[0] * k, r))));
    mat4 xM = mat4(mat3(quat(xvector(B), atan2( d[2] * k, r))));

    here.transform_orientation(transpose(xM));
    here.transform_position   (transpose(xM));
    here.transform_orientation(transpose(zM));
    here.transform_position   (transpose(zM));

    // Clamp the altitude.

    double v[3];

    here.get_position(v);
    here.set_distance(std::max(d[1] * k + r,
                            minimum_agl + sys->get_current_ground(v)));

    // Copy the modifed step back to the camera configuration.

    view_from_step(here);
}

//------------------------------------------------------------------------------

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

void orbiter::move_to(int i)
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
            {
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
            }
            sys->append_queue(new scm_step(dst));

            // Trigger playback.

            play(false);
        }
    }
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
