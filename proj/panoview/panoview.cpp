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

#include <cstring>

#include <SDL.h>
#include <SDL_keyboard.h>

#include <ogl-opengl.hpp>

#include <app-event.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-glob.hpp>

#include "panoview.hpp"

//-----------------------------------------------------------------------------

panoview::panoview(const std::string& tag) : app::prog(tag)
{
}

panoview::~panoview()
{
}

//-----------------------------------------------------------------------------

bool panoview::process_key(app::event *E)
{
//    const bool d = E->data.key.d;
//    const int  k = E->data.key.k;
//    const int  m = E->data.key.m;

    return false;
}

bool panoview::process_tick(app::event *E)
{
//    double dt = E->data.tick.dt * 0.001;

    return false;
}

bool panoview::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEY:   R = process_key (E); break;
    case E_TICK:  R = process_tick(E); break;
    }

    // Allow the application base to handle the event.

    if (R)
        return true;
    else
        return prog::process_event(E);
}

//-----------------------------------------------------------------------------

ogl::range panoview::prep(int frusc, const app::frustum *const *frusv)
{
    ogl::range r;

    return r;
}

void panoview::lite(int frusc, const app::frustum *const *frusv)
{
}

void panoview::draw(int frusi, const app::frustum *frusp)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
