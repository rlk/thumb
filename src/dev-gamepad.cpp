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

    const double kp = dt * 10;
    const double kr = dt * to_radians(45);

#if 0
    for (int i = 0; i < naxis; ++i)
        printf("%f ", axis[i]);
    for (int i = 0; i < nbutn; ++i)
        printf("%d ", butn[i]);
    printf("\n");
#endif

    quat q = ::host->get_orientation();
    mat3 R(q);

    ::host->set_orientation(q * quat(R[1], -axis[3] * kr)
                              * quat(R[0], -axis[4] * kr));


    printf("%f %f %f\n", axis[2], axis[5], (axis[5] - axis[2]) / 2);

    ::host->offset_position(mat4(q) * vec3(axis[0] * kp,
                                           axis[5] * kp
                                          -axis[2] * kp,
                                           axis[1] * kp));

    // ::host->offset_position(mat3(::host->get_orientation()) * dpad * kp);


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
