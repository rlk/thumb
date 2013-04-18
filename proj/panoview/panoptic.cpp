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

#include <cmath>

#include <ogl-opengl.hpp>

#include <etc-socket.hpp>
#include <etc-math.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "scm/util3d/math3d.h"
#include "scm/scm-log.hpp"

#include "panoptic.hpp"

//------------------------------------------------------------------------------

static in_addr_t lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)))
    {
        memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);
        return  A.s_addr;
    }
    return INADDR_NONE;
}

//------------------------------------------------------------------------------

panoptic::panoptic(const std::string& exe,
                   const std::string& tag)
    : view_app(exe, tag), now(0), delta(0)
{
    // Initialize all interaction state.

    orbit_plane[0]  = 1.0;
    orbit_plane[1]  = 0.0;
    orbit_plane[2]  = 0.0;
    orbit_speed     = 0.0;
    orbit_speed_min = ::conf->get_f("orbiter_speed_min",   0.005);
    orbit_speed_max = ::conf->get_f("orbiter_speed_max",   0.5);
    minimum_agl     = ::conf->get_f("orbiter_minimum_agl", 100.0);

    fly_up = false;
    fly_dn = false;

    stick[0] = stick[1] = 0.0;

    // Initialize the reportage socket.

    report_addr.sin_family      = AF_INET;
    report_addr.sin_port        =  htons(::conf->get_i("orbiter_report_port"));
    report_addr.sin_addr.s_addr = lookup(::conf->get_s("orbiter_report_host").c_str());

    if (report_addr.sin_addr.s_addr != INADDR_NONE)
        report_sock = socket(AF_INET, SOCK_DGRAM, 0);
    else
        report_sock = INVALID_SOCKET;

    // Initialize the joystick configuration.

    device    = ::conf->get_i("orbiter_joystick_device",    0);
    axis_X    = ::conf->get_i("orbiter_joystick_axis_X",    0);
    axis_Y    = ::conf->get_i("orbiter_joystick_axis_Y",    1);
    button_U  = ::conf->get_i("orbiter_joystick_button_U",  0);
    button_D  = ::conf->get_i("orbiter_joystick_button_D",  1);
    deadzone  = ::conf->get_f("orbiter_joystick_deadzone",  0.2);

    // Preload data as requested.

    if (char *name = getenv("SCMINIT"))
    {
        load_file(name);

        if (sys && sys->get_step_count())
            fade_to(0);
    }
}

panoptic::~panoptic()
{
    close(report_sock);
}

//------------------------------------------------------------------------------

void panoptic::report()
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
        double fly = (fly_up ? 1.0 : 0.0) - (fly_dn ? 1.0 : 0.0);

        // Encode these to an ASCII string.

        char buf[128];
        sprintf(buf, "%+12.8f %+13.8f %17.8f %+5.3f %+5.3f %+5.3f\n",
            lat, lon, alt, stick[0], stick[1], fly);

        // And send the string to the configured host.

        sendto(report_sock, buf, strlen(buf) + 1, 0,
               (const sockaddr *) &report_addr, sizeof (sockaddr_in));
    }
}

//------------------------------------------------------------------------------

void panoptic::fly(double dt)
{
    const double dx = (stick[0] > 0) ? -(stick[0] * stick[0])
                                     : +(stick[0] * stick[0]);
    const double dy = (stick[1] > 0) ? +(stick[1] * stick[1])
                                     : -(stick[1] * stick[1]);
    const double dz = fly_up ? (fly_dn ?  0 : +1)
                             : (fly_dn ? -1 :  0);

    double a = get_speed();
    double k = -M_PI_2 * lerp(std::min(1.0, cbrt(a)), 1.0, a);

    here.set_pitch(k);

    // The X axis affects the orientation and orbital plane.

    double R[16];
    double p[3];

    here.get_position(p);
    mrotate(R, p, dx * dt);
    here.transform_orientation(R);
    here.get_right(orbit_plane);

    // The Y axis affects the orbital speed.

    orbit_speed = dy * std::max(std::min(a, orbit_speed_max), orbit_speed_min);

    // The Z axis affects the orbital radius.

    double d = here.get_distance();
    double m =      get_minimum_ground();

    here.set_distance(std::min(4 * m, m + exp(log(d - m) + (dz * dt))));
}

//------------------------------------------------------------------------------

ogl::range panoptic::prep(int frusc, const app::frustum *const *frusv)
{
    report();

    view_app::prep(frusc, frusv);

    // Compute a horizon line based upon altitude and minimum terrain height.

    const double r =      get_current_ground() * get_scale();
    const double m =      get_minimum_ground() * get_scale();
    const double d = here.get_distance()       * get_scale();

    double n = 0.5 *     (d     - r    );
    double f = 1.1 * sqrt(d * d - m * m);

    return ogl::range(n, f);
}

void panoptic::draw(int frusi, const app::frustum *frusp, int chani)
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

void panoptic::load_file(const std::string& name)
{
    view_app::load_file(name);

    if (here.get_distance() == 0.0)
        here.set_distance(2.0 * get_minimum_ground());
}

double panoptic::get_speed() const
{
    const double d = here.get_distance();
    const double h =      get_current_ground();

    return (d - h) / h;
}

double panoptic::get_scale() const
{
    const double d = here.get_distance();
    const double h =      get_current_ground();

    return 1.0 / (d - h);
}

//------------------------------------------------------------------------------

void panoptic::fade_to(int i)
{
    // Construct a path from here to there.

    if (delta == 0)
    {
        if (0 <= i && i < sys->get_step_count())
        {
            // Source and destination both remain fixed.

            scm_step *src = new scm_step();
            scm_step *dst = new scm_step();

            // Change the scene to that of the requested step.

            if (sys->get_fore())
                src->set_foreground(sys->get_fore()->get_name());

            if (sys->get_back())
                src->set_background(sys->get_back()->get_name());

            dst->set_foreground(sys->get_step(i)->get_foreground());
            dst->set_background(sys->get_step(i)->get_background());

            // Queue these new steps and trigger playback.

            sys->flush_queue();
            sys->append_queue(src);
            sys->append_queue(dst);

            now   = 0;
            delta = 1.0 / 60.0;
        }
    }
}

//------------------------------------------------------------------------------

bool panoptic::process_axis(app::event *E)
{
    const int    i = E->data.axis.i;
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v / 32768;

    if (i == device)
    {
        if      (a == axis_X) stick[0] = fabs(v) < deadzone ? 0.0 : v;
        else if (a == axis_Y) stick[1] = fabs(v) < deadzone ? 0.0 : v;

        return true;
    }
    return false;
}

bool panoptic::process_button(app::event *E)
{
    const int  i = E->data.button.i;
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (i == device)
    {
        if      (b == button_D) fly_dn = d;
        else if (b == button_U) fly_up = d;

        scm_log("orbiter process_button %d %d %d", i, b, d);

        return true;
    }
    return false;
}

bool panoptic::process_tick(app::event *E)
{
    double dt = E->data.tick.dt;

    // Handle joystick input.

    fly(dt);

    // Move the position and view orientation along the current orbit.

    if (orbit_speed)
    {
        double R[16];

        mrotate(R, orbit_plane, orbit_speed * dt);

        here.transform_orientation(R);
        here.transform_position(R);
        here.transform_light(R);
    }

    // Constrain the distance using the terrain height.

    if (here.get_distance())
        here.set_distance(std::max(here.get_distance(),
                                        get_current_ground() + minimum_agl));

    // Apply the current transformation to the camera.

    double M[16];

    here.get_matrix(M);
    ::user->set_M(M);

    // Handle any scene transition.

    if (delta)
    {
        double then = now;

        if ((now = sys->set_scene_blend(now + delta)) == then)
            delta = 0;
    }
    return false;
}

bool panoptic::process_event(app::event *E)
{
    if (!view_app::process_event(E))
    {
        switch (E->get_type())
        {
            case E_AXIS:   return process_axis(E);
            case E_BUTTON: return process_button(E);
            case E_TICK:          process_tick(E);
        }
    }
    return false;
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        app::prog *P;

        P = new panoptic(argv[0], std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
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
