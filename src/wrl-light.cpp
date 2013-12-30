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

#include <wrl-light.hpp>
#include <ogl-pool.hpp>

//-----------------------------------------------------------------------------

wrl::light::light(std::string fill) : sphere(fill)
{
}


wrl::d_light::d_light() : light("light/background-basic.obj")
{
    if (fill)
        fill->set_ubiq(true); // The background is always visible
}

wrl::s_light::s_light() : light("light/spot-circle-yellow.obj")
{
}

//-----------------------------------------------------------------------------

// A spot light model is hidden during play...

void wrl::s_light::play_init()
{
    if (fill)
        fill->set_mode(false);

    sphere::play_init();
}

// ... and visible during editing.

void wrl::s_light::play_fini()
{
    if (fill)
        fill->set_mode(true);

    sphere::play_fini();
}

//-----------------------------------------------------------------------------

void wrl::light::load(app::node node)
{
    atom::load(node); // Skip solid::load.
}

void wrl::d_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "d-light");
    n.insert(node);
    atom::save(n);
}

void wrl::s_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "s-light");
    n.insert(node);
    atom::save(n);
}

//-----------------------------------------------------------------------------
