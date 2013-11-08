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

#include <wrl-world.hpp>
#include <app-view.hpp>
#include <ogl-opengl.hpp>
#include <app-frustum.hpp>
#include <mode-mode.hpp>

//-----------------------------------------------------------------------------

ogl::range mode::mode::prep(int frusc, const app::frustum *const *frusv)
{
    assert(world);

    return world->prep_fill(frusc, frusv);
}

void mode::mode::lite(int frusc, const app::frustum *const *frusv)
{
    assert(world);

    world->lite(frusc, frusv);
}

void mode::mode::draw(int frusi, const app::frustum *frusp)
{
    assert(world);

    // Draw the world.

     frusp->draw();
    ::view->draw();

    world->draw_fill(frusi, frusp);

    // HACK!  There's a state leak here.  This happens to fix it.

    ogl::line_state_init();
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------

