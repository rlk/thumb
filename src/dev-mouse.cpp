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
#include <sstream>

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
    filter(::conf->get_f("key_motion_filter", 0.9)),
    speed (::conf->get_f("key_motion_speed",  5.0)),
    mode  (::conf->get_i("key_motion_mode",   0)),

    dragging(0),
    modified(0),
    dyaw    (0),
    dpitch  (0)
{
    memset(keystate, 0, sizeof (keystate));

    parse_keyset(move_L, ::conf->get_s("key_move_L"), SDL_SCANCODE_A);
    parse_keyset(move_R, ::conf->get_s("key_move_R"), SDL_SCANCODE_D);
    parse_keyset(move_D, ::conf->get_s("key_move_D"), SDL_SCANCODE_F);
    parse_keyset(move_U, ::conf->get_s("key_move_U"), SDL_SCANCODE_R);
    parse_keyset(move_F, ::conf->get_s("key_move_F"), SDL_SCANCODE_W);
    parse_keyset(move_B, ::conf->get_s("key_move_B"), SDL_SCANCODE_S);

    parse_keyset(turn_L, ::conf->get_s("key_turn_L"), SDL_SCANCODE_Q);
    parse_keyset(turn_R, ::conf->get_s("key_turn_R"), SDL_SCANCODE_E);
    parse_keyset(turn_D, ::conf->get_s("key_turn_D"), SDL_SCANCODE_G);
    parse_keyset(turn_U, ::conf->get_s("key_turn_U"), SDL_SCANCODE_T);
}

dev::mouse::~mouse()
{
}

//-----------------------------------------------------------------------------

// Parse a comma-delimited list of numbers s, filling a vector of integers.

void dev::mouse::parse_keyset(keyset& k, const std::string& s, int d)
{
    std::stringstream str(s);
    std::string       val;

    while (std::getline(str, val, ','))
        if (int i = atoi(val.c_str()))
            k.push_back(i);

    if (k.empty()) k.push_back(d);
}

// Return 1 if any key in the given keyset is currently pressed.

int dev::mouse::check_keyset(const keyset& k) const
{
    keyset::const_iterator it;

    for (it = k.begin(); it != k.end(); ++it)
        if (keystate[*it])
            return 1;

    return 0;
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

    const vec3 npos(check_keyset(move_R) - check_keyset(move_L),
                  + check_keyset(move_U) - check_keyset(move_D),
                  + check_keyset(move_B) - check_keyset(move_F));

    const double nyaw   = check_keyset(turn_L) - check_keyset(turn_R);
    const double npitch = check_keyset(turn_U) - check_keyset(turn_D);

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
    modified = E->data.key.m & KMOD_SHIFT;

    if (0 <= E->data.key.k &&
             E->data.key.k < SDL_NUM_SCANCODES)
    {
        keystate[E->data.key.k] = E->data.key.d;
        return true;
    }

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
