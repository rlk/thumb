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

#include "orbiter.hpp"

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

orbiter::orbiter(const std::string& exe,
                 const std::string& tag)
    : view_app(exe, tag)
{
    // Initialize all interaction state.

    orbit_plane[0] = 1.0;
    orbit_plane[1] = 0.0;
    orbit_plane[2] = 0.0;
    orbit_speed    = 0.0;
    stick_timer    = 0.0;
    goto_radius    = ::conf->get_f("orbiter_goto_radius",   0.0);
    minimum_agl    = ::conf->get_f("orbiter_minimum_agl", 100.0);

    drag_move = false;
    drag_look = false;
    drag_dive = false;
    drag_lite = false;

    point[0] = point[1] = point[2] = 0.0;
    click[0] = click[1] = click[2] = 0.0;
    stick[0] = stick[1] = stick[2] = 0.0;

    // Initialize the reportage socket.

    report_addr.sin_family      = AF_INET;
    report_addr.sin_port        =  htons(::conf->get_i("orbiter_report_port"));
    report_addr.sin_addr.s_addr = lookup(::conf->get_s("orbiter_report_host").c_str());

    if (report_addr.sin_addr.s_addr != INADDR_NONE)
        report_sock = socket(AF_INET, SOCK_DGRAM, 0);
    else
        report_sock = INVALID_SOCKET;

    // Initialize the joystick configuration.

    device   = ::conf->get_i("orbiter_joystick_device", 0);
    axis_X   = ::conf->get_i("orbiter_joystick_axis_X", 0);
    axis_Y   = ::conf->get_i("orbiter_joystick_axis_Y", 1);
    button_U = ::conf->get_i("orbiter_joystick_button_U", 0);
    button_D = ::conf->get_i("orbiter_joystick_button_D", 1);
    deadzone = ::conf->get_f("orbiter_joystick_deadzone", 0.2);

    orbit_speed_min = ::conf->get_f("orbiter_speed_min", 0.005);
    orbit_speed_max = ::conf->get_f("orbiter_speed_max", 0.5);

    // Preload data as requested.

    if (char *name = getenv("SCMINIT"))
         load(name);
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

// Apply the view panning interaction, rotating left-right about the position
// vector and up-down about the current right vector.

void orbiter::look(double dt, double k)
{
    double r[3];
    double p[3];

    here.get_right(r);
    here.get_position(p);

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];
    double X[16];
    double Y[16];
    double M[16];

    mrotate(X, r,  10.0 * dy * dt);
    mrotate(Y, p, -10.0 * dx * dt);
    mmultiply(M, X, Y);

    here.transform_orientation(M);
}

void orbiter::turn(double dt, double k)
{
    double r[3];
    double p[3];
    double t[3];

    here.get_right(r);
    here.get_position(p);

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];
    double X[16];
    double Y[16];
    double M[16];

    mrotate(X, r,  10.0 * dy * dt);
    mrotate(Y, p, -10.0 * dx * dt);
    mmultiply(M, X, Y);

    here.transform_orientation(M);

    vtransform(t, Y, orbit_plane);
    vnormalize(orbit_plane, t);
}

// Apply the orbital motion interaction, using the direction of the mouse
// pointer motion to set the orbital plane and the magnitude of the mouse
// motion to set the orbital speed.

void orbiter::move(double dt, double k)
{
    if (click[0] != point[0] ||
        click[1] != point[1] ||
        click[2] != point[2])
    {
        double d = vdot(click, point);
        double a = acos(d);

        if (fabs(a) > 0.0)
        {
            const double *M = ::user->get_M();
            double t[3];
            double u[3];

            vcrs(t, click, point);

            vtransform(u, M, t);
            vnormalize(orbit_plane, u);
        }
        orbit_speed = 10.0 * a * k;
    }
    else
    {
        orbit_speed = 0.0;
    }
}

// Apply the altitude interaction using the change in the mouse pointer position
// to affect an exponential difference in the radius of the view point.

void orbiter::dive(double dt, double k)
{
    double d = (point[1] - click[1]) * dt;
    double r = here.get_distance();
    double m =      get_minimum_ground();

    r = m + exp(log(r - m) + (4 * d));

    here.set_distance(r);
}

// Apply the light position interaction, using the change in the mouse pointer
// to move the light source position vector. Light source position is relative
// to the camera, not to the sphere.

void orbiter::lite(double dt, double k)
{
    double r[3];
    double u[3];

    here.get_right(r);
    here.get_up(u);

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];
    double X[16];
    double Y[16];
    double M[16];

    mrotate(X, r, -10.0 * dy * dt);
    mrotate(Y, u,  10.0 * dx * dt);
    mmultiply(M, X, Y);

    here.transform_light(M);
}

void orbiter::fly(double dt)
{
    const double dx = (stick[0] > 0) ? -(stick[0] * stick[0])
                                     : +(stick[0] * stick[0]);
    const double dy = (stick[1] > 0) ? +(stick[1] * stick[1])
                                     : -(stick[1] * stick[1]);

    double a = get_scale();
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

    here.set_distance(std::min(4 * m, m + exp(log(d - m) + (stick[2] * dt))));
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
    double f = 1.1 * sqrt(d * d - m * m);

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

void orbiter::load(const std::string& name)
{
    view_app::load(name);

    if (here.get_distance() == 0.0)
        here.set_distance(2.0 * get_minimum_ground());
}

// Return an altitude scalar.

double orbiter::get_scale() const
{
    const double d = here.get_distance();
    const double h =      get_current_ground();

    return (d - h) / h;
}

// Construct a path from here to there with a configurable middle radius.

void orbiter::move_to(int i)
{
    // Construct a path from here to there.

    if (0 <= i && i < sys->get_step_count())
    {
        // Copy the current location and destination. Compute the middle.

        path_src = here;
        path_dst = *sys->get_step(i);
        path_mid = scm_step(&path_src, &path_dst, 0.5);

        // Tune speeds and distances. (Which is why we make copies.)

        if (goto_radius)
            path_mid.set_distance(std::max(path_mid.get_distance(), goto_radius));

        path_mid.set_pitch(-M_PI_2);
        path_mid.set_foreground(path_dst.get_foreground());
        path_mid.set_background(path_dst.get_background());

        path_src.set_speed(0.05);
        path_mid.set_speed(1.00);
        path_dst.set_speed(0.05);

        // Queue these new steps and trigger playback.

        sys->flush_queue();
        sys->append_queue(&path_src);
        sys->append_queue(&path_mid);
        sys->append_queue(&path_dst);

        orbit_speed = 0;
        now         = 0;
        dtime       = 1;
    }
}

void orbiter::fade_to(int i)
{
    // Construct a path from here to there.

    if (dtime == 0)
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
            dtime = 1;
        }
    }
}

//------------------------------------------------------------------------------

bool orbiter::process_axis(app::event *E)
{
    const int    i = E->data.axis.i;
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v;

    if (i == device)
    {
        if      (a == axis_X) stick[0] = v / 32768.0;
        else if (a == axis_Y) stick[1] = v / 32768.0;

        return true;
    }
    return false;
}

bool orbiter::process_button(app::event *E)
{
    const int  i = E->data.button.i;
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (i == device)
    {
        if      (b == button_D) stick[2] += (d ? -1 : +1);
        else if (b == button_U) stick[2] += (d ? +1 : -1);

        return true;
    }
    return false;
}

bool orbiter::process_point(app::event *E)
{
    double M[16];

    quat_to_mat(M, E->data.point.q);

    point[0] = -M[ 8];
    point[1] = -M[ 9];
    point[2] = -M[10];

    return false;
}

bool orbiter::process_click(app::event *E)
{
    const int  b = E->data.click.b;
    const int  m = E->data.click.m;
    const bool d = E->data.click.d;
    const bool s = (m & KMOD_SHIFT);
    const bool c = (m & KMOD_CTRL);

    vcpy(click, point);

    if (d)
    {
        if (b == 0)
        {
            if (s)
                drag_dive = true;
            else
                drag_move = true;
        }
        if (b == 2)
        {
            if (s)
                drag_lite = true;
            else if (c)
                drag_turn = true;
            else
                drag_look = true;
        }
    }
    else
    {
        if (b == 0) drag_dive = drag_move             = false;
        if (b == 2) drag_lite = drag_turn = drag_look = false;
    }

    return true;
}

bool orbiter::process_tick(app::event *E)
{
    double dt = E->data.tick.dt;
    double ll = vlen(stick);

    if (ll > deadzone || stick_timer > 0.0)
    {
        if (ll > deadzone)
            stick_timer = 0.1;
        else
            stick_timer -= dt;

        fly(dt);
    }
    else
    {
        double sc = get_scale();

        if (drag_move) move(dt, sc);
        if (drag_look) look(dt, sc);
        if (drag_turn) turn(dt, sc);
        if (drag_dive) dive(dt, sc);
        if (drag_lite) lite(dt, sc);
    }

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

    return false;
}

bool orbiter::process_event(app::event *E)
{
    if (!view_app::process_event(E))
    {
        switch (E->get_type())
        {
            case E_AXIS:   return process_axis(E);
            case E_BUTTON: return process_button(E);
            case E_CLICK:  return process_click(E);
            case E_POINT:  return process_point(E);
            case E_TICK:   return process_tick(E);
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
