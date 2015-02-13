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

    move_mode    (::conf->get_i("gamepad_move_mode", 1)),
    dead_zone    (::conf->get_f("gamepad_dead_zone", 0.2)),
    filter       (::conf->get_f("gamepad_filter", 0.9)),

    axis_move_R  (::conf->get_i("gamepad_axis_move_R",    0)),
    axis_move_L  (::conf->get_i("gamepad_axis_move_L",   -1)),
    axis_move_U  (::conf->get_i("gamepad_axis_move_U",    5)),
    axis_move_D  (::conf->get_i("gamepad_axis_move_D",    2)),
    axis_move_B  (::conf->get_i("gamepad_axis_move_B",    1)),
    axis_move_F  (::conf->get_i("gamepad_axis_move_F",   -1)),
    axis_turn_R  (::conf->get_i("gamepad_axis_turn_R",   -1)),
    axis_turn_L  (::conf->get_i("gamepad_axis_turn_L",    3)),
    axis_turn_U  (::conf->get_i("gamepad_axis_turn_U",   -1)),
    axis_turn_D  (::conf->get_i("gamepad_axis_turn_D",    4)),

    button_move_R(::conf->get_i("gamepad_button_move_R", -1)),
    button_move_L(::conf->get_i("gamepad_button_move_L", -1)),
    button_move_U(::conf->get_i("gamepad_button_move_U", -1)),
    button_move_D(::conf->get_i("gamepad_button_move_D", -1)),
    button_move_B(::conf->get_i("gamepad_button_move_B", -1)),
    button_move_F(::conf->get_i("gamepad_button_move_F", -1)),
    button_turn_R(::conf->get_i("gamepad_button_turn_R", -1)),
    button_turn_L(::conf->get_i("gamepad_button_turn_L", -1)),
    button_turn_U(::conf->get_i("gamepad_button_turn_U", -1)),
    button_turn_D(::conf->get_i("gamepad_button_turn_D", -1)),

    k_move_R(0),
    k_move_L(0),
    k_move_U(0),
    k_move_D(0),
    k_move_B(0),
    k_move_F(0),
    k_turn_R(0),
    k_turn_L(0),
    k_turn_U(0),
    k_turn_D(0),

    b_move_R(0),
    b_move_L(0),
    b_move_U(0),
    b_move_D(0),
    b_move_B(0),
    b_move_F(0),
    b_turn_R(0),
    b_turn_L(0),
    b_turn_U(0),
    b_turn_D(0),

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

bool dev::gamepad::process_button(app::event *E)
{
    const int    b = E->data.button.b;
    const double d = E->data.button.d ? +1.0 : -1.0;

    if (b == button_move_R) { b_move_R = d; return true; }
    if (b == button_move_L) { b_move_L = d; return true; }
    if (b == button_move_U) { b_move_U = d; return true; }
    if (b == button_move_D) { b_move_D = d; return true; }
    if (b == button_move_B) { b_move_B = d; return true; }
    if (b == button_move_F) { b_move_F = d; return true; }
    if (b == button_turn_R) { b_turn_R = d; return true; }
    if (b == button_turn_L) { b_turn_L = d; return true; }
    if (b == button_turn_U) { b_turn_U = d; return true; }
    if (b == button_turn_D) { b_turn_D = d; return true; }

    return false;
}

// Map an incoming axis change onto its configured value.

bool dev::gamepad::process_axis(app::event *E)
{
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v / 32768.0;

    if (a == axis_move_R) { k_move_R = v; return true; }
    if (a == axis_move_L) { k_move_L = v; return true; }
    if (a == axis_move_U) { k_move_U = v; return true; }
    if (a == axis_move_D) { k_move_D = v; return true; }
    if (a == axis_move_B) { k_move_B = v; return true; }
    if (a == axis_move_F) { k_move_F = v; return true; }
    if (a == axis_turn_R) { k_turn_R = v; return true; }
    if (a == axis_turn_L) { k_turn_L = v; return true; }
    if (a == axis_turn_U) { k_turn_U = v; return true; }
    if (a == axis_turn_D) { k_turn_D = v; return true; }

    return false;
}

bool dev::gamepad::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double dp = dt * 10;
    const double dr = dt * to_radians(45);

    const vec3 npos(deaden(k_move_R - k_move_L) + b_move_R - b_move_L,
                    deaden(k_move_U - k_move_D) + b_move_U - b_move_D,
                    deaden(k_move_B - k_move_F) + b_move_B - b_move_F);

    const double nyaw   = deaden(k_turn_R - k_turn_L) + b_turn_R - b_turn_L;
    const double npitch = deaden(k_turn_U - k_turn_D) + b_turn_U - b_turn_D;

    dpos   = mix(npos,   dpos,   filter);
    dyaw   = mix(nyaw,   dyaw,   filter);
    dpitch = mix(npitch, dpitch, filter);

    quat   q = ::host->get_orientation();
    double p = get_p(q) + dr * dpitch;
    double t = get_t(q) + dr * dyaw;

    p = std::max(p, -PI / 2.0);
    p = std::min(p,  PI / 2.0);

    ::host->set_orientation(quat(vec3(0, 1, 0), t)
                          * quat(vec3(1, 0, 0), p));

    if (move_mode)
        ::host->offset_position(dpos * dp);
    else
        ::host->offset_position(mat4(q) * dpos * dp);

    return false;
}

bool dev::gamepad::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_BUTTON: if (process_button(E)) return true; else break;
    case E_AXIS:   if (process_axis(E))   return true; else break;
    case E_TICK:   if (process_tick(E))   return true; else break;
    }

    return dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
