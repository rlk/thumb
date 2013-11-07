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

#include <SDL_joystick.h>

#include <etc-math.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-event.hpp>
#include <dev-gamepad.hpp>

//-----------------------------------------------------------------------------

dev::gamepad::gamepad() : button(16, false)
{
    // Defaults are arbitrarily chosen to conform to the XBox 360 controller

    gamepad_axis_X = conf->get_i("gamepad_axis_X",  0);
    gamepad_axis_Y = conf->get_i("gamepad_axis_Y",  1);
    gamepad_axis_Z = conf->get_i("gamepad_axis_Z",  2);
    gamepad_axis_T = conf->get_i("gamepad_axis_T",  3);

    gamepad_butn_L = conf->get_i("gamepad_butn_L", 13);
    gamepad_butn_R = conf->get_i("gamepad_butn_R", 12);
    gamepad_butn_D = conf->get_i("gamepad_butn_D",  8);
    gamepad_butn_U = conf->get_i("gamepad_butn_U",  9);
    gamepad_butn_F = conf->get_i("gamepad_butn_F", 14);
    gamepad_butn_B = conf->get_i("gamepad_butn_B", 11);
    gamepad_butn_H = conf->get_i("gamepad_butn_H",  4);

    gamepad_axis_X_min = conf->get_f("gamepad_axis_X_min", -32768.0);
    gamepad_axis_X_max = conf->get_f("gamepad_axis_X_max",  32767.0);
    gamepad_axis_Y_min = conf->get_f("gamepad_axis_Y_min", -32768.0);
    gamepad_axis_Y_max = conf->get_f("gamepad_axis_Y_max",  32767.0);
    gamepad_axis_Z_min = conf->get_f("gamepad_axis_Z_min", -32768.0);
    gamepad_axis_Z_max = conf->get_f("gamepad_axis_Z_max",  32767.0);
    gamepad_axis_T_min = conf->get_f("gamepad_axis_T_min", -32768.0);
    gamepad_axis_T_max = conf->get_f("gamepad_axis_T_max",  32767.0);

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;

    rotate[0] = 0;
    rotate[1] = 0;
    rotate[2] = 0;
    rotate[3] = 0;
}

//-----------------------------------------------------------------------------

double dev::gamepad::calibrate(double val, double min, double max)
{
    return 2.0 * (val - min) / (max - min) - 1.0;
}

//-----------------------------------------------------------------------------

bool dev::gamepad::process_click(app::event *E)
{
    const int  b = E->data.click.b;
    const bool d = E->data.click.d;

    const int dd = d ? +1 : -1;

    if      (b == gamepad_butn_L) { motion[0] -= dd; return true; }
    else if (b == gamepad_butn_R) { motion[0] += dd; return true; }
    else if (b == gamepad_butn_D) { motion[1] -= dd; return true; }
    else if (b == gamepad_butn_U) { motion[1] += dd; return true; }
    else if (b == gamepad_butn_F) { motion[2] -= dd; return true; }
    else if (b == gamepad_butn_B) { motion[2] += dd; return true; }
    else if (b == gamepad_butn_H) { ::user->home();  return true; }

    return false;
}

bool dev::gamepad::process_axis(app::event *E)
{
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v;

    if      (a == gamepad_axis_X)
    {
        rotate[0] = -calibrate(v, gamepad_axis_X_min, gamepad_axis_X_max);
        return true;
    }
    else if (a == gamepad_axis_Y)
    {
        rotate[1] = +calibrate(v, gamepad_axis_Y_min, gamepad_axis_Y_max);
        return true;
    }
    else if (a == gamepad_axis_Z)
    {
        rotate[2] = -calibrate(v, gamepad_axis_Z_min, gamepad_axis_Z_max);
        return true;
    }
    else if (a == gamepad_axis_T)
    {
        rotate[3] = -calibrate(v, gamepad_axis_T_min, gamepad_axis_T_max);
        return true;
    }

    return false;
}

bool dev::gamepad::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;

    const double kp =        dt;
    const double kr = 45.0 * dt;

    const bool   bx = (fabs(rotate[0]) > 0.25);
    const bool   by = (fabs(rotate[1]) > 0.25);
    const bool   bz = (fabs(rotate[2]) > 0.25);

    const bool   bp = (DOT3(motion, motion) != 0);
    const bool   br = bx || by || bz;

    if (bp) ::user->move(motion[0] * kp, motion[1] * kp, motion[2] * kp);
    if (br) ::user->turn(rotate[1] * kr, rotate[0] * kr, rotate[2] * kr);

    return false;
}

bool dev::gamepad::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_CLICK: R |= process_click(E); break;
    case E_AXIS:  R |= process_axis(E); break;
    case E_TICK:  R |= process_tick(E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
