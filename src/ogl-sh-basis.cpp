//  Copyright (C) 2009 Robert Kooima
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

#include "app-glob.hpp"
#include "app-conf.hpp"
#include "ogl-frame.hpp"
#include "ogl-program.hpp"
#include "ogl-sh-basis.hpp"

//-----------------------------------------------------------------------------

ogl::sh_basis::sh_basis(const std::string& name, int i) :
    process(name),

    prog(::glob->load_program("irr/sh-basis.xml")),

    cube(::glob->new_frame(::conf->get_i("reflection_cubemap_size", 128),
                           ::conf->get_i("reflection_cubemap_size", 128),
                           GL_TEXTURE_CUBE_MAP,
                           GL_RGBA32F_ARB, true, false, false)),
    index(i)
{
    init();
}

ogl::sh_basis::~sh_basis()
{
    assert(prog);
    assert(cube);

    ::glob->free_frame(cube);
    ::glob->free_program(prog);
}

//-----------------------------------------------------------------------------

void ogl::sh_basis::bind(GLenum unit) const
{
    assert(cube);

    cube->bind_color(unit);
}

void ogl::sh_basis::init()
{
    assert(prog);
    assert(cube);

    prog->bind();
    prog->uniform("l", 0);
    prog->uniform("m", 0);

    proc_cube(cube);
}

//-----------------------------------------------------------------------------
