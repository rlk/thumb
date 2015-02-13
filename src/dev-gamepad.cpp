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

    axis_move_R(::conf->get_i("gamepad_axis_move_R",  0)),
    axis_move_L(::conf->get_i("gamepad_axis_move_L", -1)),
    axis_move_U(::conf->get_i("gamepad_axis_move_U",  5)),
    axis_move_D(::conf->get_i("gamepad_axis_move_D",  2)),
    axis_move_B(::conf->get_i("gamepad_axis_move_B",  1)),
    axis_move_F(::conf->get_i("gamepad_axis_move_F", -1)),
    axis_turn_R(::conf->get_i("gamepad_axis_turn_R", -1)),
    axis_turn_L(::conf->get_i("gamepad_axis_turn_L",  3)),
    axis_turn_U(::conf->get_i("gamepad_axis_turn_U", -1)),
    axis_turn_D(::conf->get_i("gamepad_axis_turn_D",  4)),

    value_move_R(0),
    value_move_L(0),
    value_move_U(0),
    value_move_D(0),
    value_move_B(0),
    value_move_F(0),
    value_turn_R(0),
    value_turn_L(0),
    value_turn_U(0),
    value_turn_D(0),

    dx    (0),
    dy    (0),
    dz    (0),
    dyaw  (0),
    dpitch(0)
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

    if (a == axis_move_R) { value_move_R = v; return true; }
    if (a == axis_move_L) { value_move_L = v; return true; }
    if (a == axis_move_U) { value_move_U = v; return true; }
    if (a == axis_move_D) { value_move_D = v; return true; }
    if (a == axis_move_B) { value_move_B = v; return true; }
    if (a == axis_move_F) { value_move_F = v; return true; }
    if (a == axis_turn_R) { value_turn_R = v; return true; }
    if (a == axis_turn_L) { value_turn_L = v; return true; }
    if (a == axis_turn_U) { value_turn_U = v; return true; }
    if (a == axis_turn_D) { value_turn_D = v; return true; }

    return false;
}

bool dev::gamepad::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double dp = dt * 10;
    const double dr = dt * to_radians(45);

    const double nx     = deaden(value_move_R - value_move_L);
    const double ny     = deaden(value_move_U - value_move_D);
    const double nz     = deaden(value_move_B - value_move_F);
    const double nyaw   = deaden(value_turn_R - value_turn_L);
    const double npitch = deaden(value_turn_U - value_turn_D);

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
