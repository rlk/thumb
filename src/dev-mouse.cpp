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
    speed(5.0),
    dragging(false),
    modifier(0)
{
    key_move_F = conf->get_i("key_move_F", SDL_SCANCODE_W);
    key_move_L = conf->get_i("key_move_L", SDL_SCANCODE_A);
    key_move_R = conf->get_i("key_move_R", SDL_SCANCODE_D);
    key_move_B = conf->get_i("key_move_B", SDL_SCANCODE_S);
    key_move_U = conf->get_i("key_move_U", SDL_SCANCODE_E);
    key_move_D = conf->get_i("key_move_D", SDL_SCANCODE_Q);
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
        dragging = bool(d);
        return true;
    }
    return false;
}

bool dev::mouse::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

    int dd = d ? +1 : -1;

    modifier = m;

    // Handle motion keys.

    if      (k == key_move_L) { motion[0] -= dd; return true; }
    else if (k == key_move_R) { motion[0] += dd; return true; }
    else if (k == key_move_D) { motion[1] -= dd; return true; }
    else if (k == key_move_U) { motion[1] += dd; return true; }
    else if (k == key_move_F) { motion[2] -= dd; return true; }
    else if (k == key_move_B) { motion[2] += dd; return true; }

    // Teleport home. Allow other layers to respond by NOT claiming the event.

    else if (d)
    {
        if (k == SDL_SCANCODE_HOME)
        {
            ::view->go_home();
            return false;
        }
    }

    return false;
}

bool dev::mouse::process_tick(app::event *E)
{
    double kp = E->data.tick.dt * speed;

    if (modifier & KMOD_SHIFT) kp *= 10.0;

    if (motion * motion > 0)
        ::host->offset_position(mat3(::host->get_orientation()) * motion * kp);

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
