//  Copyright (C) 2005 Robert Kooima
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

#include "main.hpp"
#include "app-event.hpp"
#include "wrl-world.hpp"
#include "mode-play.hpp"

//-----------------------------------------------------------------------------

mode::play::play(wrl::world *w) : mode(w)
{
}

//-----------------------------------------------------------------------------
// START and CLOSE events are generated whenever a mode transition occurs.

bool mode::play::process_start(app::event *E)
{
    assert(world);

    world->play_init();
    clr_time();

    return false;
}

bool mode::play::process_close(app::event *E)
{
    assert(world);

    world->play_fini();

    return false;
}

bool mode::play::process_timer(app::event *E)
{
    assert(E);
    assert(world);

    world->play_step(E->data.timer.dt * 0.001);

    return false;
}

//-----------------------------------------------------------------------------

bool mode::play::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_START: R |= process_start(E); break;
    case E_CLOSE: R |= process_close(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || mode::process_event(E);
}

//-----------------------------------------------------------------------------
