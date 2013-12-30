//  Copyright (C) 2009-2011 Robert Kooima
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

#include <app-glob.hpp>
#include <app-conf.hpp>
#include <ogl-frame.hpp>
#include <ogl-binding.hpp>
#include <ogl-reflection-env.hpp>

//-----------------------------------------------------------------------------

ogl::reflection_env::reflection_env(const std::string& name, int i) :
    process(name),

    cube(::glob->new_frame(::conf->get_i("reflection_cubemap_size", 128),
                           ::conf->get_i("reflection_cubemap_size", 128),
                           GL_TEXTURE_CUBE_MAP,
                           GL_RGBA16F, true, false, false))
{
    init();
}

ogl::reflection_env::~reflection_env()
{
    assert(cube);

    ::glob->free_frame(cube);
}

//-----------------------------------------------------------------------------

void ogl::reflection_env::draw(const ogl::binding *calc)
{
    assert(cube);

    if (calc->bind(true))
        proc_cube(cube);
}

void ogl::reflection_env::bind(GLenum unit) const
{
    assert(cube);

    cube->bind_color(unit);
}

//-----------------------------------------------------------------------------
