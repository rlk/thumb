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
#include "ogl-process.hpp"
#include "ogl-binding.hpp"
#include "ogl-irradiance-env.hpp"

//-----------------------------------------------------------------------------

ogl::irradiance_env::irradiance_env() :
    process("irradiance-env"),

    rsize(::conf->get_i("reflection_cubemap_size", 128)),
    isize(::conf->get_i("irradiance_cubemap_size", 128)),

    d(::glob->load_process("d-omega")),
    L(::glob->load_process("reflection-env")),

    init(::glob->load_program("irradiance-init", "irradiance-init")),
    step(::glob->load_program("irradiance-sum",  "irradiance-sum")),
    calc(::glob->load_program("irradiance-calc", "irradiance-calc")),

    ping(::glob->new_frame(rsize, rsize, GL_TEXTURE_2D,
                           GL_RGBA32F_ARB, true, false, false)),
    pong(::glob->new_frame(rsize, rsize, GL_TEXTURE_2D,
                           GL_RGBA32F_ARB, true, false, false)),
    cube(::glob->new_frame(isize, isize, GL_TEXTURE_CUBE_MAP,
                           GL_RGBA16F_ARB, true, false, false))
{
}

ogl::irradiance_env::~irradiance_env()
{
    ::glob->free_process(d);
    ::glob->free_process(L);

    ::glob->free_program(init);
    ::glob->free_program(step);
    ::glob->free_program(calc);

    ::glob->free_frame(ping);
    ::glob->free_frame(pong);
    ::glob->free_frame(cube);
}

//-----------------------------------------------------------------------------

void ogl::irradiance_env::draw(const ogl::binding *bind) const
{
    
}

//-----------------------------------------------------------------------------

void ogl::irradiance_env::bind(GLenum unit) const
{
    assert(cube);

    cube->bind_color(unit);
}

//-----------------------------------------------------------------------------
