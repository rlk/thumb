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
#include "app-conf.hpp"
#include "app-user.hpp"
#include "dev-gamepad.hpp"

//-----------------------------------------------------------------------------

dev::gamepad::gamepad(uni::universe& universe) :
    universe(universe)
{
    gamepad_axis_X = conf->get_i("gamepad_axis_X");
    gamepad_axis_Y = conf->get_i("gamepad_axis_Y");
    gamepad_axis_Z = conf->get_i("gamepad_axis_Z");
    gamepad_axis_A = conf->get_i("gamepad_axis_A");
    gamepad_axis_T = conf->get_i("gamepad_axis_T");

    gamepad_butn_L = conf->get_i("gamepad_butn_L");
    gamepad_butn_R = conf->get_i("gamepad_butn_R");
    gamepad_butn_D = conf->get_i("gamepad_butn_D");
    gamepad_butn_U = conf->get_i("gamepad_butn_U");
    gamepad_butn_F = conf->get_i("gamepad_butn_F");
    gamepad_butn_B = conf->get_i("gamepad_butn_B");

    gamepad_butn_H = conf->get_i("gamepad_butn_H");

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;

    rotate[0] = 0;
    rotate[1] = 0;
    rotate[2] = 0;
    rotate[3] = 0;
    rotate[4] = 0;

    button.reserve(16);
}

dev::gamepad::~gamepad()
{
}

//-----------------------------------------------------------------------------

bool dev::gamepad::click(int i, int b, int m, bool d)
{
    int dd = d ? +1 : -1;

    if      (b == gamepad_butn_L) { motion[0] -= dd; return true; }
    else if (b == gamepad_butn_R) { motion[0] += dd; return true; }
    else if (b == gamepad_butn_D) { motion[1] -= dd; return true; }
    else if (b == gamepad_butn_U) { motion[1] += dd; return true; }
    else if (b == gamepad_butn_F) { motion[2] -= dd; return true; }
    else if (b == gamepad_butn_B) { motion[2] += dd; return true; }

    else if (b == gamepad_butn_H) { ::user->home(); return true; }

    return false;
}

bool dev::gamepad::value(int d, int a, double v)
{
    if      (a == gamepad_axis_X) { rotate[0] = -v; return true; }
    else if (a == gamepad_axis_Y) { rotate[1] =  v; return true; }
    else if (a == gamepad_axis_Z) { rotate[2] = -v; return true; }
    else if (a == gamepad_axis_A) { rotate[3] =  v; return true; }
    else if (a == gamepad_axis_T) { rotate[4] = -v; return true; }

    return false;
}

bool dev::gamepad::timer(int t)
{
    double dt = t / 1000.0;

    double kr = dt * view_turn_rate;
    double kp = dt * universe.move_rate();
    double ka = dt * universe.turn_rate();

    bool bx = (fabs(rotate[0]) > 0.1);
    bool by = (fabs(rotate[1]) > 0.1);
    bool bz = (fabs(rotate[2]) > 0.1);
    bool ba = (fabs(rotate[3]) > 0.1);
    bool bt = (fabs(rotate[4]) > 0.1);

    user->move(motion[0] * kp, motion[1] * kp, motion[2] * kp);

    if (bx || by || bz)
        user->turn(rotate[1] * kr, rotate[0] * kr, rotate[2] * kr);

    if (ba || bt)
        universe.set_time(universe.get_time() + dt * ka);

    if (bx || by || bz || ba || bt || DOT3(motion, motion) > 0)
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------
