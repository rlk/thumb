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
#include "ogl-binding.hpp"
#include "ogl-d-omega.hpp"

//-----------------------------------------------------------------------------

ogl::d_omega::d_omega() :
    process("d_omega"),

    calc(::glob->load_binding("d-omega", "d-omega")),
    cube(::glob->new_frame(::conf->get_i("reflection_cubemap_size", 128),
                           ::conf->get_i("reflection_cubemap_size", 128),
                           GL_TEXTURE_CUBE_MAP,
                           GL_RGBA32F_ARB, true, false, false))
{
    init();
}

ogl::d_omega::~d_omega()
{
    assert(calc);
    assert(cube);

    ::glob->free_frame(cube);
    ::glob->free_binding(calc);
}

//-----------------------------------------------------------------------------

void ogl::d_omega::bind(GLenum unit) const
{
    assert(cube);

    cube->bind_color(unit);
}

void ogl::d_omega::init()
{
    assert(calc);
    assert(cube);

    proc_cube(calc, cube);
}

//-----------------------------------------------------------------------------
