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

#include <cassert>
#include <algorithm>

#include <SDL_joystick.h>

#include <etc-vector.hpp>
#include <app-conf.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-gamepad.hpp>

//-----------------------------------------------------------------------------

// Default gamepade configuration fits an XBox 360 controller (without roll).

dev::gamepad::gamepad() :

    dead_zone(0.2),
    filter   (0.9),

    axis_pos_x     (::conf->get_i("gamepad_axis_pos_x",      0)),
    axis_neg_x     (::conf->get_i("gamepad_axis_neg_x",     -1)),
    axis_pos_y     (::conf->get_i("gamepad_axis_pos_y",      5)),
    axis_neg_y     (::conf->get_i("gamepad_axis_neg_y",      2)),
    axis_pos_z     (::conf->get_i("gamepad_axis_pos_z",      1)),
    axis_neg_z     (::conf->get_i("gamepad_axis_neg_z",     -1)),
    axis_pos_yaw   (::conf->get_i("gamepad_axis_pos_yaw",   -1)),
    axis_neg_yaw   (::conf->get_i("gamepad_axis_neg_yaw",    3)),
    axis_pos_pitch (::conf->get_i("gamepad_axis_pos_pitch", -1)),
    axis_neg_pitch (::conf->get_i("gamepad_axis_neg_pitch",  4)),
    axis_pos_roll  (::conf->get_i("gamepad_axis_pos_roll ", -1)),
    axis_neg_roll  (::conf->get_i("gamepad_axis_neg_roll ", -1)),

    value_pos_x    (0),
    value_neg_x    (0),
    value_pos_y    (0),
    value_neg_y    (0),
    value_pos_z    (0),
    value_neg_z    (0),
    value_pos_yaw  (0),
    value_neg_yaw  (0),
    value_pos_pitch(0),
    value_neg_pitch(0),
    value_pos_roll (0),
    value_neg_roll (0),

    dx    (0),
    dy    (0),
    dz    (0),
    dyaw  (0),
    dpitch(0),
    droll (0)
{
}

double dev::gamepad::deaden(double k) const
{
    if      (k > +dead_zone) return k + dead_zone * (k - 1);
    else if (k < -dead_zone) return k + dead_zone * (k + 1);
    else                     return 0;
}

//-----------------------------------------------------------------------------

static double get_p(const quat& q)
{
    const vec3 z = zvector(mat3(q));

    if (z[1] >=  1.0) return -PI / 2.0;
    if (z[1] <= -1.0) return  PI / 2.0;

    return asin(-z[1]);
}

static double get_t(const quat& q)
{
    const vec3 x = xvector(mat3(q));
    return atan2(-x[2], x[0]);
}

//-----------------------------------------------------------------------------

// Map an incoming axis change onto its configured value.

bool dev::gamepad::process_axis(app::event *E)
{
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v / 32768.0;

    if (a == axis_pos_x)     { value_pos_x     = v; return true; }
    if (a == axis_neg_x)     { value_neg_x     = v; return true; }
    if (a == axis_pos_y)     { value_pos_y     = v; return true; }
    if (a == axis_neg_y)     { value_neg_y     = v; return true; }
    if (a == axis_pos_z)     { value_pos_z     = v; return true; }
    if (a == axis_neg_z)     { value_neg_z     = v; return true; }
    if (a == axis_pos_yaw)   { value_pos_yaw   = v; return true; }
    if (a == axis_neg_yaw)   { value_neg_yaw   = v; return true; }
    if (a == axis_pos_pitch) { value_pos_pitch = v; return true; }
    if (a == axis_neg_pitch) { value_neg_pitch = v; return true; }
    if (a == axis_pos_roll)  { value_pos_roll  = v; return true; }
    if (a == axis_neg_roll)  { value_neg_roll  = v; return true; }

    return false;
}

bool dev::gamepad::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double dp = dt * 10;
    const double dr = dt * to_radians(45);

    const double nx     = deaden(value_pos_x     - value_neg_x);
    const double ny     = deaden(value_pos_y     - value_neg_y);
    const double nz     = deaden(value_pos_z     - value_neg_z);
    const double nyaw   = deaden(value_pos_yaw   - value_neg_yaw);
    const double npitch = deaden(value_pos_pitch - value_neg_pitch);

    dx     = filter * dx     + (1.0 - filter) * nx;
    dy     = filter * dy     + (1.0 - filter) * ny;
    dz     = filter * dz     + (1.0 - filter) * nz;
    dyaw   = filter * dyaw   + (1.0 - filter) * nyaw;
    dpitch = filter * dpitch + (1.0 - filter) * npitch;

    quat   q = ::host->get_orientation();
    double p = get_p(q) + dr * dpitch;
    double t = get_t(q) + dr * dyaw;

    p = std::max(p, -PI / 2.0);
    p = std::min(p,  PI / 2.0);

    ::host->set_orientation(quat(vec3(0, 1, 0), t)
                          * quat(vec3(1, 0, 0), p));

    ::host->offset_position(mat4(q) * vec3(dp * dx, dp * dy, dp * dz));

    return false;
}

bool dev::gamepad::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_AXIS: if (process_axis(E)) return true; else break;
    case E_TICK: if (process_tick(E)) return true; else break;
    }

    return dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
