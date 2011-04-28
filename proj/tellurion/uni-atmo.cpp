//  Copyright (C) 2007 Robert Kooima
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

#include "app-glob.hpp"
#include "uni-atmo.hpp"

//-----------------------------------------------------------------------------

uni::atmo::atmo(double r0, double r1) : r0(r0), r1(r1),

    atmo_in (glob->load_program("glsl/uni/SkyFromAtmosphere.vert",
                                "glsl/uni/SkyFromAtmosphere.frag")),
    atmo_out(glob->load_program("glsl/uni/SkyFromSpace.vert",
                                "glsl/uni/SkyFromSpace.frag")),
    land_in (glob->load_program("glsl/uni/GroundFromAtmosphere.vert",
                                "glsl/uni/GroundFromAtmosphere.frag")),
    land_out(glob->load_program("glsl/uni/GroundFromSpace.vert",
                                "glsl/uni/GroundFromSpace.frag"))
{
    pool = glob->new_pool();
    node = new ogl::node();
    unit = new ogl::unit("solid/inverted_sphere.obj");

    pool->add_node(node);
    node->add_unit(unit);
}

uni::atmo::~atmo()
{
}

//-----------------------------------------------------------------------------

void uni::atmo::draw()
{
}

//-----------------------------------------------------------------------------
