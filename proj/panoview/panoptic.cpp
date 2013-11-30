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
#include <etc-vector.hpp>
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
    : view_app(exe, tag), now(0), delta(0), index(0)
{
    // Initialize all interaction state.

    orbiter_speed_min   = ::conf->get_f("orbiter_speed_min", 0.0005);
    orbiter_speed_max   = ::conf->get_f("orbiter_speed_max", 0.5);
    orbiter_minimum_agl = ::conf->get_f("orbiter_minimum_agl", 100.0);

    memset(axis,   0, sizeof (axis));
    memset(button, 0, sizeof (button));

    // Initialize the reportage socket.

    report_addr.sin_family      = AF_INET;
    report_addr.sin_port        =  htons(::conf->get_i("orbiter_report_port"));
    report_addr.sin_addr.s_addr = lookup(::conf->get_s("orbiter_report_host").c_str());

    if (report_addr.sin_addr.s_addr != INADDR_NONE)
        report_sock = socket(AF_INET, SOCK_DGRAM, 0);
    else
        report_sock = INVALID_SOCKET;

    // Initialize the joystick configuration.

    device                   = ::conf->get_i("joystick_device",    0);
    deadzone                 = ::conf->get_f("joystick_deadzone",  0.2);

    orbiter_axis_rotate      = ::conf->get_i("orbiter_axis_rotate",       0);
    orbiter_axis_forward     = ::conf->get_i("orbiter_axis_forward",      1);
    orbiter_axis_left        = ::conf->get_i("orbiter_axis_left",         2);
    orbiter_axis_right       = ::conf->get_i("orbiter_axis_right",        5);
    orbiter_button_up        = ::conf->get_i("orbiter_joystick_button_U", 0);
    orbiter_button_down      = ::conf->get_i("orbiter_joystick_button_D", 1);

    panoview_axis_vertical   = ::conf->get_i("panoview_axis_vertical",    4);
    panoview_axis_horizontal = ::conf->get_i("panoview_axis_horizontal",  3);
    panoview_button_in       = ::conf->get_i("panoview_button_in",        4);
    panoview_button_out      = ::conf->get_i("panoview_button_out",       5);
    panoview_zoom_min        = ::conf->get_f("panoview_zoom_min",       0.5);
    panoview_zoom_max        = ::conf->get_f("panoview_zoom_max",       3.0);

    scene_button_next        = ::conf->get_f("scene_button_next",         2);
    scene_button_prev        = ::conf->get_f("scene_button_prev",         3);

    // Preload data as requested.

    if (char *name = getenv("SCMINIT"))
    {
        load_file(name);

        if (sys->get_scene_count())
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
        double fly = (button[orbiter_button_up]   ? 1.0 : 0.0)
                   - (button[orbiter_button_down] ? 1.0 : 0.0);

        // Encode these to an ASCII string.

        char buf[128];
        sprintf(buf, "%+12.8f %+13.8f %17.8f %+5.3f %+5.3f %+5.3f\n",
            lat, lon, alt, axis[orbiter_axis_rotate],
                           axis[orbiter_axis_forward], fly);

        // And send the string to the configured host.

        sendto(report_sock, buf, strlen(buf) + 1, 0,
               (const sockaddr *) &report_addr, sizeof (sockaddr_in));
    }
}

//------------------------------------------------------------------------------

double panoptic::deaden(double k) const
{
    if (fabs(k) < deadzone)
        return 0;
    else
        return (k < 0) ? -k * k : +k * k;
}

void panoptic::joystick(double dt)
{
    const double y[3] = { 0.0, 1.0, 0.0 };

    const double dh = deaden(axis[panoview_axis_horizontal]);
    const double dv = deaden(axis[panoview_axis_vertical]);
    const double dz = deaden(axis[orbiter_axis_forward]);
    const double dr = deaden(axis[orbiter_axis_rotate]);
    const double dx = deaden(axis[orbiter_axis_right] -
                             axis[orbiter_axis_left]);

    double dy  = button[orbiter_button_up]
              ? (button[orbiter_button_down] ?  0 : +1)
              : (button[orbiter_button_down] ? -1 :  0);
    double dm  = button[panoview_button_in]
              ? (button[panoview_button_out] ?  0 : +1)
              : (button[panoview_button_out] ? -1 :  0);

    double s  = get_speed();
    double ds = std::max(std::min(s, orbiter_speed_max), orbiter_speed_min);

    double M[16];
    double v[3];

    // Orbiter turning rotation

    if (fabs(dr) > 0)
    {
        here.get_position(v);
        mrotate(M, v, -dr * dt);
        here.transform_orientation(M);
        here.transform_light(M);
    }

    // Orbiter motion forward and back

    if (fabs(dz) > 0)
    {
        here.get_right(v);
        mrotate(M, v, dz * ds * dt);
        here.transform_orientation(M);
        here.transform_position(M);
        here.transform_light(M);
    }

    // Orbiter motion left and right

    if (fabs(dx) > 0)
    {
        here.get_forward(v);
        mrotate(M, v, dx * ds * dt);
        here.transform_orientation(M);
        here.transform_position(M);
        here.transform_light(M);
    }

    // Orbiter motion up and down

    if (fabs(dy) > 0)
    {
        double k = -M_PI_2 * lerp(std::min(1.0, cbrt(s)), 1.0, s);
        double d = here.get_distance();
        double m =      get_current_ground();

        here.set_distance(std::min(8 * m, m + exp(log(d - m) + (dy * dt))));
        here.set_pitch(k);
    }

    // Panoview pan left and right

    if (fabs(dh) > 0)
    {
        mrotate(M, y, -dh * dt);
        here.transform_orientation(M);
        here.transform_position(M);
        here.transform_light(M);
    }

    // Panoview tilt up and down

    if (fabs(dv) > 0)
    {
        here.get_right(v);
        mrotate(M, v, -dv * dt);
        here.transform_orientation(M);
        here.transform_position(M);
        here.transform_light(M);
    }

    // Panoview zoom in and out

    if (fabs(dm) > 0)
    {
        double z = here.get_zoom() + dm * dt;
        z = std::min(z, panoview_zoom_max);
        z = std::max(z, panoview_zoom_min);
        here.set_zoom(z);
    }

    if (fabs(dm) > 0 || fabs(dh) > 0 || fabs(dv) > 0)
    {
        here.get_forward(v);
        sys->get_sphere()->set_zoom(v[0], v[1], v[2], here.get_zoom());
    }
}

//------------------------------------------------------------------------------

ogl::range panoptic::prep(int frusc, const app::frustum *const *frusv)
{
    double M[16];

    report();

    view_app::prep(frusc, frusv);

    here.get_matrix(M);
    ::view->set_move_matrix(M);

    // Compute a horizon line based upon altitude and minimum terrain height.

    const double r =      get_current_ground() * get_scale();
    const double m =      get_minimum_ground() * get_scale();
    const double d = here.get_distance()       * get_scale();

    double n = 0.1 *     (d     - r    );
    double f = 1.1 * sqrt(d * d - m * m);

    return ogl::range(n, f);
}

void panoptic::lite(int frusc, const app::frustum *const *frusv)
{
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

int panoptic::fade_to(int i)
{
    if (sys->get_step_count())
    {
        int m = sys->get_step_count() - 1;

        if (i > m) i = 0;
        if (i < 0) i = m;

        // Source and destination both remain fixed.

        scm_step *src = new scm_step(here);
        scm_step *dst = new scm_step(here);

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

        if (delta == 0)
            now    = 0;

        delta = 1.0 / 16.0;

        return i;
    }
    return 0;
}

//------------------------------------------------------------------------------

bool panoptic::process_axis(app::event *E)
{
    const int    i = E->data.axis.i;
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v / 32768.0;

    if (i == device)
    {
        if (0 <= a && a < 8)
        {
            axis[a] = v;
            return true;
        }
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
        if (0 <= b && b < 16)
        {
            button[b] = d;

            if (d && b == scene_button_next) index = fade_to(index + 1);
            if (d && b == scene_button_prev) index = fade_to(index - 1);

            return true;
        }
    }
    return false;
}

bool panoptic::process_tick(app::event *E)
{
    double dt = E->data.tick.dt;

    // Handle joystick input.

    joystick(dt);

    // Constrain the distance using the terrain height.

    if (here.get_distance())
        here.set_distance(std::max(here.get_distance(),
                  orbiter_minimum_agl + get_current_ground()));

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
