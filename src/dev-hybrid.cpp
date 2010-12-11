//  Copyright (C) 2010 Robert Kooima
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

#include "matrix.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-event.hpp"
#include "dev-hybrid.hpp"

//-----------------------------------------------------------------------------

dev::hybrid::hybrid()
{
    // Defaults are arbitrarily chosen to conform to the XBox 360 controller

    pos_axis_LR = conf->get_i("hybrid_pos_axis_LR", 0);
    pos_axis_FB = conf->get_i("hybrid_pos_axis_FB", 1);
    rot_axis_LR = conf->get_i("hybrid_rot_axis_LR", 2);
    rot_axis_UD = conf->get_i("hybrid_rot_axis_UD", 3);

    button_A    = conf->get_i("hybrid_button_A", 0);
    button_H    = conf->get_i("hybrid_button_H", 5);

    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    rotation[0] = 0;
    rotation[1] = 0;
}

//-----------------------------------------------------------------------------

bool dev::hybrid::process_click(app::event *E)
{
    const int  i = E->data.click.i;
    const int  b = E->data.click.b;
    const int  m = E->data.click.m;
    const bool d = E->data.click.d;
    const int dd = d ? +1 : -1;

    printf("process_click %d %d %d %d\n", i, b, m, d);

    if (b == button_H) { ::user->home();  return true; }

    return false;
}

bool dev::hybrid::process_value(app::event *E)
{
    const int    a = E->data.value.a;
    const double v = E->data.value.v / 32767.0;

    printf("process_value %d %f\n", a, v);

    if      (a == pos_axis_LR) { position[0] = +v; return true; }
    else if (a == pos_axis_FB) { position[2] = +v; return true; }
    else if (a == 4)           { position[1] = -(v + 1.0) * 0.5; return true; }
    else if (a == 5)           { position[1] = +(v + 1.0) * 0.5; return true; }
    else if (a == rot_axis_LR) { rotation[0] = -v; return true; }
    else if (a == rot_axis_UD) { rotation[1] = -v; return true; }

    return false;
}

bool dev::hybrid::process_timer(app::event *E)
{
    const double dt = E->data.timer.dt * 0.001;
    const double dz = 0.25;

    const double kp = dt * ::user->get_move_rate();
    const double kr = dt * ::user->get_turn_rate();
    const double kt = 3.0 * 60.0 * 60.0 * dt;

    printf("%f %f\n", ::user->get_move_rate(), ::user->get_turn_rate());

    if (fabs(position[0]) > dz ||
        fabs(position[1]) > dz ||
        fabs(position[2]) > dz) ::user->move(position[0] * kp,
                                             position[1] * kp,
                                             position[2] * kp);

    if (fabs(rotation[0]) > dz ||
        fabs(rotation[1]) > dz) ::user->look(rotation[0] * kr,
                                             rotation[1] * kr);

    if (0)
        ::user->pass(kt);

    return false;
}

bool dev::hybrid::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_CLICK: R |= process_click(E); break;
    case E_VALUE: R |= process_value(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
