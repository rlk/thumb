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

#include <etc-vector.hpp>
#include <app-conf.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-gamepad.hpp>

//-----------------------------------------------------------------------------

dev::gamepad::gamepad()
{
    for (int i = 0; i < naxis; i++) { gamepad_axis[i] = i; axis[i] = 0; }
    for (int i = 0; i < nbutn; i++) { gamepad_butn[i] = i; butn[i] = 0; }
}

//-----------------------------------------------------------------------------

bool dev::gamepad::process_button(app::event *E)
{
    const int b = E->data.button.b;
    const int d = E->data.button.d;

    for (int i = 0; i < 16; ++i)
        if (b == gamepad_butn[i])
        {
            butn[i] = d;
            return true;
        }

    return false;
}

bool dev::gamepad::process_axis(app::event *E)
{
    const int    a = E->data.axis.a;
    const double v = E->data.axis.v / 32768.0;

    for (int i = 0; i < 16; ++i)
        if (a == gamepad_axis[i])
        {
            axis[i] = v;
            return true;
        }

    return false;
}

bool dev::gamepad::process_tick(app::event *E)
{
    const double dt = E->data.tick.dt;

    const double kp =        dt;
    const double kr = 45.0 * dt;

    // const bool bx = (fabs(axis[0]) > 0.1);
    // const bool by = (fabs(axis[1]) > 0.1);
    // const bool bz = (fabs(axis[2]) > 0.1);
    // const bool   br = bx || by || bz;

    for (int i = 0; i < naxis; ++i)
        printf("%f ", axis[i]);
    for (int i = 0; i < nbutn; ++i)
        printf("%d ", butn[i]);
    printf("\n");

        // ::host->offset_position(mat3(::host->get_orientation()) * dpad * kp);

    // if (bp) ::view->move(dpad[0] * kp, dpad[1] * kp, dpad[2] * kp);
    // if (br) ::view->turn(axis[1] * kr, axis[0] * kr, axis[2] * kr);

    return false;
}

bool dev::gamepad::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_BUTTON: R |= process_button(E); break;
    case E_AXIS:   R |= process_axis  (E); break;
    case E_TICK:   R |= process_tick  (E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
