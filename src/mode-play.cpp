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

#include <cassert>

#include "main.hpp"
#include "app-conf.hpp"
#include "app-prog.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "app-event.hpp"
#include "app-frustum.hpp"
#include "wrl-world.hpp"
#include "mode-play.hpp"

//-----------------------------------------------------------------------------

mode::play::play(wrl::world *w) :
    mode(w),
    count(0),
    movie(::conf->get_i("movie"))
{
}

mode::play::~play()
{
}

//-----------------------------------------------------------------------------

void mode::play::draw(int frusi, app::frustum *frusp)
{
    assert(world);

    // Draw the world.

     frusp->draw();
    ::user->draw();

    world->draw_fill(frusi, frusp);

    // Count frames and record a movie, if requested.
        
    if (movie)
    {
        count++;

        if ((count % movie) == 0)
        {
            char buf[256];

            sprintf(buf, "frame%05d.png", count / movie);

            ::prog->screenshot(std::string(buf),
                               ::host->get_window_w(),
                               ::host->get_window_h());
        }
    }
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
