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

#include "binding.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

ogl::binding::binding(std::string name) : name(name)
{
    lite = glob->load_program("object-lite.vert", "object-lite-frag");
    dark = glob->load_program("object-dark.vert", "object-dark-frag");

    diff = glob->load_texture("solid/metal_box_diffuse.png");
    bump = glob->load_texture("solid/metal_box_bump.png");

    lite->bind();
    {
        lite->uniform("diffuse", 0);
        lite->uniform("bump",    1);
        lite->uniform("light",   2);
        lite->uniform("shadow",  3);
    }
    lite->free();

    dark->bind();
    {
        dark->uniform("diffuse", 0);
    }
    dark->free();
}

ogl::binding::~binding()
{
    glob->free_texture(bump);
    glob->free_texture(diff);
    glob->free_program(dark);
    glob->free_program(lite);
}

//-----------------------------------------------------------------------------

void ogl::binding::bind(bool lit) const
{
    if (lit)
        lite->bind();
    else
        dark->bind();

    diff->bind(GL_TEXTURE0);
    bump->bind(GL_TEXTURE1);
}

//-----------------------------------------------------------------------------
