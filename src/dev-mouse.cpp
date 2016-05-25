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
#include <cstdlib>

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <etc-vector.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-mouse.hpp>

//-----------------------------------------------------------------------------

dev::mouse::mouse() :
    move_L(::conf->get_s("key_move_L"), SDL_SCANCODE_A),
    move_R(::conf->get_s("key_move_R"), SDL_SCANCODE_D),
    move_D(::conf->get_s("key_move_D"), SDL_SCANCODE_F),
    move_U(::conf->get_s("key_move_U"), SDL_SCANCODE_R),
    move_F(::conf->get_s("key_move_F"), SDL_SCANCODE_W),
    move_B(::conf->get_s("key_move_B"), SDL_SCANCODE_S),

    turn_L(::conf->get_s("key_turn_L"), SDL_SCANCODE_Q),
    turn_R(::conf->get_s("key_turn_R"), SDL_SCANCODE_E),
    turn_D(::conf->get_s("key_turn_D"), SDL_SCANCODE_G),
    turn_U(::conf->get_s("key_turn_U"), SDL_SCANCODE_T),

    filter(::conf->get_f("key_motion_filter", 0.9)),
    speed (::conf->get_f("key_motion_speed",  5.0)),
    mode  (::conf->get_i("key_motion_mode",   0)),

    dragging(0),
    modified(0),
    dyaw    (0),
    dpitch  (0)
{
}

dev::mouse::~mouse()
{
}

//-----------------------------------------------------------------------------

// Calculate the phi angle of the mouselook. Clamp at straight up and straight
// down. In theory |z[1]| shouldn't exceed 1, but in practice it does, which is
// numerically catastrophic.

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

bool dev::mouse::process_point(app::event *E)
{
    last_q = curr_q;
    curr_q = quat(E->data.point.q[0],
                  E->data.point.q[1],
                  E->data.point.q[2],
                  E->data.point.q[3]);

    if (dragging)
    {
        const quat q = ::host->get_orientation();

        double p = get_p(q) + 2.0 * (get_p(curr_q) - get_p(last_q));
        double t = get_t(q) + 2.0 * (get_t(curr_q) - get_t(last_q));

        p = std::max(p, -PI / 2.0);
        p = std::min(p,  PI / 2.0);

        ::host->set_orientation(quat(vec3(0, 1, 0), t)
                              * quat(vec3(1, 0, 0), p));
        return true;
    }
    return false;
}

bool dev::mouse::process_click(app::event *E)
{
    const int d = E->data.click.d;
    const int b = E->data.click.b;

    // Handle rotating the view.

    if (b == SDL_BUTTON_RIGHT)
    {
        dragging = d ? true : false;
        return true;
    }
    return false;
}

bool dev::mouse::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double dr = dt * to_radians(45);
    const double dp = dt * speed * (modified ? 10.0 : 1.0);

    // Calculate position and orientation differentials.

    const vec3 npos(move_R.check() - move_L.check(),
                  + move_U.check() - move_D.check(),
                  + move_B.check() - move_F.check());

    const double nyaw   = turn_L.check() - turn_R.check();
    const double npitch = turn_U.check() - turn_D.check();

    // Filter the input differentials.

    dpos   = mix(npos,   dpos,   filter);
    dyaw   = mix(nyaw,   dyaw,   filter);
    dpitch = mix(npitch, dpitch, filter);

    // Compute and apply the new orientation.

    quat   q = ::host->get_orientation();
    double p = get_p(q) + dr * dpitch;
    double t = get_t(q) + dr * dyaw;

    p = std::max(p, -PI / 2.0);
    p = std::min(p,  PI / 2.0);

    ::host->set_orientation(quat(vec3(0, 1, 0), t)
                          * quat(vec3(1, 0, 0), p));

    // Compute and apply the change in position.

    if (mode)
        ::host->offset_position(dpos * dp);
    else
        ::host->offset_position(mat4(q) * dpos * dp);

    return false;
}

bool dev::mouse::process_key(app::event *E)
{
    const int  k = E->data.key.k;
    const bool d = E->data.key.d;

    modified = E->data.key.m & KMOD_SHIFT;

    return (move_L.event(k, d)
         || move_R.event(k, d)
         || move_D.event(k, d)
         || move_U.event(k, d)
         || move_F.event(k, d)
         || move_B.event(k, d)
         || turn_L.event(k, d)
         || turn_R.event(k, d)
         || turn_D.event(k, d)
         || turn_U.event(k, d));
}

bool dev::mouse::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_POINT: if (process_point(E)) return true; else break;
    case E_CLICK: if (process_click(E)) return true; else break;
    case E_TICK:  if (process_tick (E)) return true; else break;
    case E_KEY:   if (process_key  (E)) return true; else break;
    }
    return dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
