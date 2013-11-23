//  Copyright (C) 2013 Robert Kooima
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

#include <etc-vector.hpp>
#include <app-default.hpp>
#include <app-conf.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-sixense.hpp>

#include <sixense.h>

//-----------------------------------------------------------------------------

dev::sixense::sixense() :
    status(false),
    flying(false)
{
    if ((status = (sixenseInit()           == SIXENSE_SUCCESS &&
                   sixenseSetActiveBase(0) == SIXENSE_SUCCESS)))
    {
        float nr = ::conf->get_f("sixense_filter_near_range", 500.00);
        float nv = ::conf->get_f("sixense_filter_near_value",   0.95);
        float fr = ::conf->get_f("sixense_filter_far_range", 1600.00);
        float fv = ::conf->get_f("sixense_filter_far_value",    0.99);

        sixenseSetFilterParams(nr, nv, fr, fv);
        sixenseSetFilterEnabled(1);
    }

    move_rate       = ::conf->get_f("sixense_move_rate",  30.0);
    turn_rate       = ::conf->get_f("sixense_turn_rate", 120.0);
    hand_controller = ::conf->get_i("sixense_hand_controller", 0);
    fly_button      = ::conf->get_i("sixense_fly_button     ", 1);
}

dev::sixense::~sixense()
{
    if (status) sixenseExit();
}

//-----------------------------------------------------------------------------

void dev::sixense::translate()
{
    sixenseAllControllerData curr;

    if (status && sixenseGetAllNewestData(&curr) == SIXENSE_SUCCESS)
    {
        sixenseControllerData *a = curr.controllers + hand_controller;
        sixenseControllerData *b = prev.controllers + hand_controller;

        app::event E;

        // Generate motion events.

        if (memcmp(a->pos,      b->pos,      3 * sizeof (float)) ||
            memcmp(a->rot_quat, b->rot_quat, 4 * sizeof (float)))
        {
            double p[3];
            double q[4];

            p[0] = a->pos[0] / 1000.0;
            p[1] = a->pos[1] / 1000.0;
            p[2] = a->pos[2] / 1000.0;

            q[0] = a->rot_quat[0];
            q[1] = a->rot_quat[1];
            q[2] = a->rot_quat[2];
            q[3] = a->rot_quat[3];

            ::host->process_event(E.mk_point(1, p, q));
        }

        // Generate joystick events.

        if (a->joystick_x != b->joystick_x)
            ::host->process_event(E.mk_axis(1, 0, a->joystick_x));

        if (a->joystick_y != b->joystick_y)
            ::host->process_event(E.mk_axis(1, 1, a->joystick_y));

        if (a->trigger != b->trigger)
            ::host->process_event(E.mk_axis(1, 2, a->trigger));

        // Generate button events.

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_START)
            ::host->process_event(E.mk_button(1, 0, a->buttons & SIXENSE_BUTTON_START));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_BUMPER)
            ::host->process_event(E.mk_button(1, 1, a->buttons & SIXENSE_BUTTON_BUMPER));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_JOYSTICK)
            ::host->process_event(E.mk_button(1, 2, a->buttons & SIXENSE_BUTTON_JOYSTICK));

        // Generate click events.

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_1)
            ::host->process_event(E.mk_click(1, 0, a->buttons & SIXENSE_BUTTON_1));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_2)
            ::host->process_event(E.mk_click(2, 0, a->buttons & SIXENSE_BUTTON_2));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_3)
            ::host->process_event(E.mk_click(3, 0, a->buttons & SIXENSE_BUTTON_3));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_4)
            ::host->process_event(E.mk_click(4, 0, a->buttons & SIXENSE_BUTTON_4));

        prev = curr;
    }
}

//-----------------------------------------------------------------------------

bool dev::sixense::process_point(app::event *E)
{
    const int     i = E->data.point.i;
    const double *p = E->data.point.p;
    const double *q = E->data.point.q;

    if (i == 1)
    {
        curr_p = vec3(p[0], p[1], p[2]);
        curr_q = quat(q[0], q[1], q[2], q[3]);
    }
    return false;
}

bool dev::sixense::process_axis(app::event *E)
{
//  const int    a = E->data.axis.a;
//  const double v = E->data.axis.v;

    return false;
}

bool dev::sixense::process_button(app::event *E)
{
    const int  i = E->data.button.i;
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (i == 1 && b == fly_button)
    {
        flying = d;
        init_p = curr_p;
        init_q = curr_q;

        return true;
    }
    return false;
}

bool dev::sixense::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;

    if (::host->root())
        translate();

    if (flying)
    {
        quat q = normal(inverse(init_q) * curr_q);
        mat4 R = mat4(mat3(slerp(quat(), q, 1.0 / 30.0)));
        mat4 T = translation((curr_p - init_p) * dt * move_rate);

        ::host->navigate(T * R);
    }
    else
        ::host->navigate(mat4());

    return false;
}

bool dev::sixense::process_event(app::event *E)
{
    switch (E->get_type())
    {
    case E_POINT:  if (process_point (E)) return true; else break;
    case E_AXIS:   if (process_axis  (E)) return true; else break;
    case E_BUTTON: if (process_button(E)) return true; else break;
    case E_TICK:   if (process_tick  (E)) return true; else break;
    }
    return dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
