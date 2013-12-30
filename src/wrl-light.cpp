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

wrl::light::light(std::string name) : sphere(name, false)
{
}


wrl::d_light::d_light(std::string name) : light(name)
{
    params[param::brightness] = new param("brightness", "1.0");
}

wrl::s_light::s_light(std::string name) : light(name)
{
    params[param::brightness] = new param("brightness", "1.0");
    params[param::falloff]    = new param("falloff",    "0.0");
    params[param::cutoff]     = new param("cutoff",    "90.0");
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
    // Load the OBJ file.

    if (app::node n = node.find("file"))
    {
        name = n.get_s();
        fill = new ogl::unit(name, false);
        fill->set_ubiq(true);
    }
    atom::load(node); // Skip solid::load.
}

void wrl::d_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "d-light");
    n.insert(node);
    solid::save(n);
}

void wrl::s_light::save(app::node node)
{
    app::node n("light");

    n.set_s("type", "s-light");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
