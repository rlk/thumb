//  Copyright (C) 2005-2011 Robert Kooima
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

#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <wrl-world.hpp>
#include <mode-play.hpp>

//-----------------------------------------------------------------------------

mode::play::play(wrl::world *w) : mode(w)
{
}

mode::play::~play()
{
}

//-----------------------------------------------------------------------------
// START and CLOSE events are generated whenever a mode transition occurs.

bool mode::play::process_start(app::event *E)
{
    assert(world);

    world->play_init();

    return false;
}

bool mode::play::process_close(app::event *E)
{
    assert(world);

    world->play_fini();

    return false;
}

bool mode::play::process_tick(app::event *E)
{
    assert(E);
    assert(world);

    world->play_step(E->data.tick.dt);

    return false;
}

//-----------------------------------------------------------------------------

bool mode::play::process_event(app::event *E)
{
    assert(E);

    switch (E->get_type())
    {
    case E_START: if (process_start(E)) return true; else break;
    case E_CLOSE: if (process_close(E)) return true; else break;
    case E_TICK:  if (process_tick (E)) return true; else break;
    }
    return mode::process_event(E);
}

//-----------------------------------------------------------------------------
