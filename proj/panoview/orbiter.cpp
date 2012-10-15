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

#include <etc-math.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "util3d/math3d.h"

#include "orbiter.hpp"

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
    goto_radius    = ::conf->get_f("orbiter_goto_radius", 0.0);

    control   = false;
    drag_move = false;
    drag_look = false;
    drag_dive = false;
    drag_lite = false;

    point[0] = point[1] = point[2] = 0.0;
    click[0] = click[1] = click[2] = 0.0;
    stick[0] = stick[1] = stick[2] = 0.0;

    // Initialize the reportage socket.

    report_addr.sin_family      = AF_INET;
    report_addr.sin_port        =     htons(::conf->get_i("orbiter_report_port"));
    report_addr.sin_addr.s_addr = inet_addr(::conf->get_s("orbiter_report_host").c_str());

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

    if (model)
    {
        if (report_addr.sin_addr.s_addr != INADDR_NONE &&
            report_sock                 != INVALID_SOCKET)
        {
            // Compute the current longitude, latitude, and altitude.

            double p[3], alt = here.get_radius();

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
    double r = here.get_radius();
    double m =      get_bottom();

    r = m + exp(log(r - m) + (4 * d));

    here.set_radius(r);
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

    mrotate(X, r,  10.0 * dy * dt);
    mrotate(Y, u, -10.0 * dx * dt);
    mmultiply(M, X, Y);

    here.transform_light(M);
}

void orbiter::fly(double dt)
{
    const double dx = (stick[0] > 0) ? -(stick[0] * stick[0])
                                     : +(stick[0] * stick[0]);
    const double dy = (stick[1] > 0) ? +(stick[1] * stick[1])
                                     : -(stick[1] * stick[1]);

    double a = (here.get_radius() - get_bottom()) / get_bottom();

    double k = lerp(std::min(1.0, cbrt(a)), 1.0, a);

    here.set_pitch(-M_PI_2 * k);

    // The X axis affects the orientation and orbital plane.

    double R[16];
    double p[3];

    here.get_position(p);
    mrotate(R, p, dx * dt);
    here.transform_orientation(R);
    here.get_right(orbit_plane);

    // The Y axis affects the orbital speed.

    orbit_speed = dy * std::min(a, 2.0);

    // The Z axis affects the orbital radius.

    double r = here.get_radius();
    double m =      get_bottom();
    double e =      get_radius();

    here.set_radius(std::min(4.0 * e, m + exp(log(r - m) + (stick[2] * dt))));
}

//------------------------------------------------------------------------------

ogl::range orbiter::prep(int frusc, const app::frustum *const *frusv)
{
    report();

    view_app::prep(frusc, frusv);

    if (model)
    {
        // Compute a horizon line based upon altitude and  minimum data radius.

        double a = here.get_radius();
        double r =      get_scale(a) * get_bottom();
        double d =      get_scale(a) * a;

        double n = 0.0001 *     (d     - r    );
        double f = 1.1    * sqrt(d * d - r * r);

        return ogl::range(n, f);
    }
    return ogl::range(0.1, 10.0);
}

void orbiter::draw(int frusi, const app::frustum *frusp, int chani)
{
    if (gui_state)
        glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
    else
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double  l[3];
    GLfloat L[4];

    here.get_light(l);

    L[0] = GLfloat(l[0]);
    L[1] = GLfloat(l[1]);
    L[2] = GLfloat(l[2]);
    L[3] = 0.0f;

    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, L);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);

    glFrontFace(GL_CW);
    view_app::draw(frusi, frusp, chani);

    glFrontFace(GL_CCW);
    view_app::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

bool orbiter::process_event(app::event *E)
{
    if (!view_app::process_event(E))
    {
        switch (E->get_type())
        {
            case E_AXIS:   return pan_axis(E);
            case E_BUTTON: return pan_button(E);
            case E_CLICK:  return pan_click(E);
            case E_POINT:  return pan_point(E);
            case E_TICK:   return pan_tick(E);
        }
    }

    return false;
}

void orbiter::load(const std::string& name)
{
    view_app::load(name);

    if (here.get_radius() == 0.0)
        here.set_radius(2.0 * get_radius());
}

// Return the effective altitude of the given radius: the height above the
// lowest point on the planet.

double orbiter::get_bottom() const
{
    if (bound)
        return get_radius() * bound->get_r0();
    else
        return get_radius();
}

// Return an appropriate planet scale coefficient for a given view radius.
// That allows good stereoscopic 3D to be achieved at both near and far views.

double orbiter::get_scale(double r) const
{
    return 1.0 / (r - get_bottom());
}

// Construct a path from here to there with a configurable middle radius.

void orbiter::make_path(int i)
{
    view_step src = here;
    view_step dst = steps[i];
    view_step mid(&src, &dst, 0.5);

    if (!path.playing())
        src.set_speed(0.05);

    if (goto_radius)
        mid.set_radius(goto_radius);

    mid.set_speed(1.00);
    dst.set_speed(0.05);

    path.add(src);
    path.add(mid);
    path.add(dst);
}

//------------------------------------------------------------------------------

bool orbiter::pan_axis(app::event *E)
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

bool orbiter::pan_button(app::event *E)
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

bool orbiter::pan_point(app::event *E)
{
    double M[16];

    // Determine the point direction of the given quaternion.

    quat_to_mat(M, E->data.point.q);

    point[0] = -M[ 8];
    point[1] = -M[ 9];
    point[2] = -M[10];

    return false;
}

bool orbiter::pan_click(app::event *E)
{
    const int  b = E->data.click.b;
    const int  m = E->data.click.m;
    const bool d = E->data.click.d;
    const bool s = (m & KMOD_SHIFT);
    const bool c = (m & KMOD_CTRL);

    control = c;

    vcpy(click, point);

    if (d)
    {
        if (b == 0)
        {
            // if      (c)
            //     drag_turn = true;
            if (s)
                drag_look = true;
            else
                drag_move = true;
        }
        if (b == 2)
        {
            if (s)
                drag_lite = true;
            else
                drag_dive = true;
        }
    }
    else
    {
        if (b == 0) drag_look = drag_turn = drag_move = false;
        if (b == 2) drag_lite = drag_dive             = false;
    }

    return true;
}

bool orbiter::pan_tick(app::event *E)
{
    double dt = E->data.tick.dt / 1000.0;
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
        double sc = 1.0 / (get_radius() * get_scale(here.get_radius()));

        if (control) sc *= 0.1;

        if (drag_move) move(dt, sc);
        if (drag_look) look(dt, sc);
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

    // Apply the current transformation to the camera.

    double M[16];

    here.get_matrix(M, get_scale(here.get_radius()));
    ::user->set_M(M);

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
