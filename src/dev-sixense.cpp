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

#include <etc-math.hpp>
#include <app-default.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-sixense.hpp>

#include <sixense.h>

//-----------------------------------------------------------------------------

dev::sixense::sixense() :
    status(false)
{
    status = (sixenseInit()           == SIXENSE_SUCCESS &&
              sixenseSetActiveBase(0) == SIXENSE_SUCCESS);

    move_rate       = ::conf->get_f("sixense_move_rate", 5.0);
    turn_rate       = ::conf->get_f("sixense_turn_rate", 60.0);
    hand_controller = ::conf->get_i("sixense_hand_controller", 0);
}

dev::sixense::~sixense()
{
    if (status) sixenseExit();
}

//-----------------------------------------------------------------------------

void dev::sixense::translate()
{
    sixenseAllControllerData curr;

    if (sixenseGetAllNewestData(&curr) == SIXENSE_SUCCESS)
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

            ::host->process_event(E.mk_point(0, p, q));
        }

        // Generate joystick events.

        if (a->joystick_x != b->joystick_x)
            ::host->process_event(E.mk_axis(0, 0, a->joystick_x));

        if (a->joystick_y != b->joystick_y)
            ::host->process_event(E.mk_axis(0, 1, a->joystick_y));

        if (a->trigger != b->trigger)
            ::host->process_event(E.mk_axis(0, 2, a->trigger));

        // Generate button events.

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_START)
            ::host->process_event(E.mk_button(0, 0, a->buttons & SIXENSE_BUTTON_START));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_1)
            ::host->process_event(E.mk_button(0, 1, a->buttons & SIXENSE_BUTTON_1));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_2)
            ::host->process_event(E.mk_button(0, 2, a->buttons & SIXENSE_BUTTON_2));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_3)
            ::host->process_event(E.mk_button(0, 3, a->buttons & SIXENSE_BUTTON_3));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_4)
            ::host->process_event(E.mk_button(0, 4, a->buttons & SIXENSE_BUTTON_4));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_BUMPER)
            ::host->process_event(E.mk_button(0, 5, a->buttons & SIXENSE_BUTTON_BUMPER));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_JOYSTICK)
            ::host->process_event(E.mk_button(0, 6, a->buttons & SIXENSE_BUTTON_JOYSTICK));

        prev = curr;
    }
}

//-----------------------------------------------------------------------------

bool dev::sixense::process_point(app::event *E)
{
    const int     i = E->data.point.i;
    const double *p = E->data.point.p;
    const double *q = E->data.point.q;

    double R[16];

    quat_to_mat(R, q);

    if (i == 0)
        printf("process_point %f %f %f %f %f %f\n",
                p[0], p[1], p[2], R[8], R[9], R[10]);

    return false;
}

bool dev::sixense::process_axis(app::event *E)
{
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v;

    printf("process_axis %d %f\n", a, v);

    return false;
}

bool dev::sixense::process_button(app::event *E)
{
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    printf("process_button %d %d\n", b, d);

    return false;
}

bool dev::sixense::process_tick(app::event *E)
{
    // const double dt = E->data.tick.dt;

    if (::host->root())
        translate();

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
