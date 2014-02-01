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

wrl::light::light(app::node node, std::string name) : sphere(node, name)
{
}


wrl::d_light::d_light(app::node node, std::string name) : light(node, name)
{
    if (fill)
        fill->set_ubiq(true);

    params[param::brightness]  = new param("brightness",  "1.0");

    load_params(node);
}

wrl::s_light::s_light(app::node node, std::string name) : light(node, name)
{
    params[param::brightness]  = new param("brightness",  "1.0");
    params[param::attenuation] = new param("attenuation", "0.0");
    params[param::cutoff]      = new param("cutoff",     "90.0");

    load_params(node);
}

//-----------------------------------------------------------------------------

// Extract and return directional lighting parameters.

double wrl::d_light::get_lighting(vec2& brightness) const
{
    param_map::const_iterator i;

    double a =  0.0;
    double b =  1.0;
    double c = 90.0;

    if ((i = params.find(param::brightness)) != params.end())
        b = double(i->second->value());

    brightness = vec2(b, a);
    return c;
}

// Extract and return spot lighting parameters.

double wrl::s_light::get_lighting(vec2& brightness) const
{
    param_map::const_iterator i;

    double a =  0.0;
    double b =  1.0;
    double c = 90.0;

    if ((i = params.find(param::attenuation)) != params.end())
        a = double(i->second->value());

    if ((i = params.find(param::brightness))  != params.end())
        b = double(i->second->value());

    if ((i = params.find(param::cutoff))      != params.end())
        c = double(i->second->value());

    brightness = vec2(b, a);
    return c;
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
