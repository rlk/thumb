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
    key_move_L(::conf->get_i("key_move_L", SDL_SCANCODE_A)),
    key_move_R(::conf->get_i("key_move_R", SDL_SCANCODE_D)),
    key_move_D(::conf->get_i("key_move_D", SDL_SCANCODE_F)),
    key_move_U(::conf->get_i("key_move_U", SDL_SCANCODE_R)),
    key_move_F(::conf->get_i("key_move_F", SDL_SCANCODE_W)),
    key_move_B(::conf->get_i("key_move_B", SDL_SCANCODE_S)),

    key_turn_L(::conf->get_i("key_turn_L", SDL_SCANCODE_Q)),
    key_turn_R(::conf->get_i("key_turn_R", SDL_SCANCODE_E)),
    key_turn_D(::conf->get_i("key_turn_D", SDL_SCANCODE_G)),
    key_turn_U(::conf->get_i("key_turn_U", SDL_SCANCODE_T)),

    filter(::conf->get_f("key_motion_filter", 0.9)),
    speed (::conf->get_f("key_motion_speed",  5.0)),
    mode  (::conf->get_i("key_motion_mode",   0)),

    dragging(0),
    modifier(0),

    move_L(0),
    move_R(0),
    move_D(0),
    move_U(0),
    move_F(0),
    move_B(0),

    turn_L(0),
    turn_R(0),
    turn_D(0),
    turn_U(0),

    dyaw  (0),
    dpitch(0)
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

bool dev::mouse::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

    int dd = d ? 1 : 0;

    modifier = m;

    // Handle motion keys.

    if (k == key_move_L) { move_L = dd; return true; }
    if (k == key_move_R) { move_R = dd; return true; }
    if (k == key_move_D) { move_D = dd; return true; }
    if (k == key_move_U) { move_U = dd; return true; }
    if (k == key_move_F) { move_F = dd; return true; }
    if (k == key_move_B) { move_B = dd; return true; }

    if (k == key_turn_L) { turn_L = dd; return true; }
    if (k == key_turn_R) { turn_R = dd; return true; }
    if (k == key_turn_D) { turn_D = dd; return true; }
    if (k == key_turn_U) { turn_U = dd; return true; }

    // Teleport home.

    else if (d)
    {
        if (k == SDL_SCANCODE_HOME)
        {
            ::view->go_home();
            return false;
        }
    }

    //  Allow other layers to respond by not claiming the event.

    return false;
}

bool dev::mouse::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double dp = dt * speed * ((modifier & KMOD_SHIFT) ? 10.0 : 1.0);
    const double dr = dt * to_radians(45);

    // Calculate position and orientation differentials.

    const vec3 npos(move_R - move_L,
                  + move_U - move_D,
                  + move_B - move_F);

    const double nyaw   = turn_L - turn_R;
    const double npitch = turn_U - turn_D;

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
