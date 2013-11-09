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
    if ((status = (sixenseInit()           == SIXENSE_SUCCESS &&
                   sixenseSetActiveBase(0) == SIXENSE_SUCCESS)))
    {
        float nr = ::conf->get_f("sixense_filter_near_range", 500.00);
        float nv = ::conf->get_f("sixense_filter_near_value",   0.95);
        float fr = ::conf->get_f("sixense_filter_far_range", 1600.00);
        float fv = ::conf->get_f("sixense_filter_far_value",    0.99);

        sixenseSetFilterParams(nr, 0.95, fr, 0.99);
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

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_BUMPER)
            ::host->process_event(E.mk_button(0, 1, a->buttons & SIXENSE_BUTTON_BUMPER));

        if ((a->buttons ^ b->buttons) & SIXENSE_BUTTON_JOYSTICK)
            ::host->process_event(E.mk_button(0, 2, a->buttons & SIXENSE_BUTTON_JOYSTICK));

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

    if (i == hand_controller)
    {
        curr_P[0] = p[0];
        curr_P[1] = p[1];
        curr_P[2] = p[2];

        quat_to_mat(curr_R, q);
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
    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    if (b == fly_button)
    {
        flying = d;

        init_P[0] = curr_P[0];
        init_P[1] = curr_P[1];
        init_P[2] = curr_P[2];

        load_mat(init_R, curr_R);

        return true;
    }
    return false;
}

bool dev::sixense::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;
    const double kr = dt * turn_rate;
    const double kp = dt * move_rate;

    if (::host->root())
        translate();

    if (flying)
    {
        double dP[3];
        double dR[3];
        double dz[3];
        double dy[3];

        dP[0] = curr_P[ 0] - init_P[ 0];
        dP[1] = curr_P[ 1] - init_P[ 1];
        dP[2] = curr_P[ 2] - init_P[ 2];

        dy[0] = init_R[ 4] - curr_R[ 4];
        dy[1] = init_R[ 5] - curr_R[ 5];
        dy[2] = init_R[ 6] - curr_R[ 6];

        dz[0] = init_R[ 8] - curr_R[ 8];
        dz[1] = init_R[ 9] - curr_R[ 9];
        dz[2] = init_R[10] - curr_R[10];

        dR[0] =  DOT3(dz, init_R + 4);
        dR[1] = -DOT3(dz, init_R + 0);
        dR[2] =  DOT3(dy, init_R + 0);

        ::view->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
        ::view->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
    }

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