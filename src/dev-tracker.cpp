//  Copyright (C) 2007 Robert Kooima
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

#include <SDL_joystick.h>

#include "matrix.hpp"
#include "app-prog.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "dev-tracker.hpp"

//-----------------------------------------------------------------------------

dev::tracker::tracker(uni::universe& universe) :
    universe(universe)
{
    tracker_head_sensor = conf->get_i("tracker_head_sensor");
    tracker_hand_sensor = conf->get_i("tracker_hand_sensor");

    tracker_butn_fly    = conf->get_i("tracker_butn_fly");
    tracker_butn_home   = conf->get_i("tracker_butn_home");

    tracker_axis_A      = conf->get_i("tracker_axis_A");
    tracker_axis_T      = conf->get_i("tracker_axis_T");

    view_move_rate      = conf->get_f("view_move_rate");
    view_turn_rate      = conf->get_f("view_turn_rate");

    rotate[0] = 0;
    rotate[1] = 0;

    init_P[0] = curr_P[0];
    init_P[1] = curr_P[1];
    init_P[2] = curr_P[2];

    load_idt(init_R);
    load_idt(curr_R);

    button.reserve(3);
}

dev::tracker::~tracker()
{
}

//-----------------------------------------------------------------------------

bool dev::tracker::point(int i, const double *p, const double *q)
{
    if (i == tracker_head_sensor)
        ::host->set_head(p, q);

    if (i == tracker_hand_sensor)
    {
        curr_P[0] = p[0];
        curr_P[1] = p[1];
        curr_P[2] = p[2];

        set_quaternion(curr_R, q);

        if (button[tracker_butn_fly])
            return true;
    }

    return false;
}

bool dev::tracker::click(int i, int b, int m, bool d)
{
    button[b] = d;

    if (b == tracker_butn_fly)
    {
        init_P[0] = curr_P[0];
        init_P[1] = curr_P[1];
        init_P[2] = curr_P[2];

        load_mat(init_R, curr_R);
    }
//  else if (b == 1 && d) ::user->home();
    else if (b == 1 && !d) { ::prog->trigger(1); return false; } // HACK
    else if (b == 2 && d) ::prog->toggle(2);

    return true;
}

bool dev::tracker::value(int d, int a, double v)
{
    if      (a == tracker_axis_A) { rotate[0] = v; return true; }
    else if (a == tracker_axis_T) { rotate[1] = v; return true; }

    return false;
}

bool dev::tracker::timer(int t)
{
    double dt = t / 1000.0;

    double kr = dt * view_turn_rate;
    double kp = dt * universe.move_rate();
    double ka = dt * universe.turn_rate();

    // Handle navigation.

    if (button[tracker_butn_fly])
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

        user->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
        user->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
    }

    // Handle planet rotation.

    bool ba = (fabs(rotate[0]) > 0.1);
    bool bt = (fabs(rotate[1]) > 0.1);

    if (ba || bt)
        universe.turn(rotate[0] * ka, rotate[1] * ka);

    if (ba || bt || button[tracker_butn_fly])
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------
