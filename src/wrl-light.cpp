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

wrl::light::light(std::string fill) : atom(fill, "wire/wire_sphere.obj")
{
    edit_geom = dCreateSphere(0, dReal(1.0));

    line_scale[0] = line_scale[1] = line_scale[2] = 1.0;

    dGeomSetData(edit_geom, this);
    ode_set_geom_transform(edit_geom, current_M);
}


wrl::d_light::d_light() : light("solid/d-light.obj")
{
    params[GL_RED]                   = new param("color_red",             "1.0");
    params[GL_GREEN]                 = new param("color_green",           "1.0");
    params[GL_BLUE]                  = new param("color_blue",            "1.0");
}

wrl::s_light::s_light() : light("solid/s-light.obj")
{
    params[GL_RED]                   = new param("color_red",             "1.0");
    params[GL_GREEN]                 = new param("color_green",           "1.0");
    params[GL_BLUE]                  = new param("color_blue",            "1.0");
    params[GL_CONSTANT_ATTENUATION]  = new param("attenuation_constant",  "1.0");
    params[GL_LINEAR_ATTENUATION]    = new param("attenuation_linear",    "0.0");
    params[GL_QUADRATIC_ATTENUATION] = new param("attenuation_quadratic", "0.0");
    params[GL_SPOT_EXPONENT]         = new param("spot_exponent",         "8.0");
    params[GL_SPOT_CUTOFF]           = new param("spot_cutoff",          "30.0");
}

//-----------------------------------------------------------------------------

void wrl::light::play_init()
{
    if (fill) fill->set_mode(false);
}

void wrl::light::play_fini()
{
    if (fill) fill->set_mode(true);
}

//-----------------------------------------------------------------------------

void wrl::d_light::save(app::node node)
{
    app::node n("light");

    n.set_s("class", "d_light");
    n.insert(node);
    atom::save(n);
}

void wrl::s_light::save(app::node node)
{
    app::node n("light");

    n.set_s("class", "s_light");
    n.insert(node);
    atom::save(n);
}

//-----------------------------------------------------------------------------
