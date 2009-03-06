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

#include "ogl-pool.hpp"
#include "ogl-frame.hpp"
#include "ogl-program.hpp"
#include "ogl-process.hpp"
#include "ogl-binding.hpp"
#include "ogl-irradiance-env.hpp"

//-----------------------------------------------------------------------------

ogl::irradiance_env::irradiance_env() :
    process("irradiance-env"),

    b(::conf->get_i("spherical-harmonic-order", 2)),
    n(::conf->get_i("reflection_cubemap_size", 128)),
    m(::conf->get_i("irradiance_cubemap_size", 128)),

    d(::glob->load_process("d_omega")),
    L(::glob->load_process("reflection_env")),

    init(::glob->load_program("irr/irradiance-init.xml")),
    step(::glob->load_program("irr/irradiance-sum.xml")),
    calc(::glob->load_program("irr/irradiance-calc.xml")),

    ping(::glob->new_frame(n * (b + 1), n * (b + 1), GL_TEXTURE_RECTANGLE_ARB,
                           GL_RGBA32F_ARB, true, false, false)),
    pong(::glob->new_frame(n * (b + 1), n * (b + 1), GL_TEXTURE_RECTANGLE_ARB,
                           GL_RGBA32F_ARB, true, false, false)),
    cube(::glob->new_frame(m, m, GL_TEXTURE_CUBE_MAP,
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
    assert(ping);
    assert(init);

    ping->bind();
    {
        init->bind();

        clip_pool->prep();
        clip_pool->draw_init();
        {
            clip_node->draw();
        }
        clip_pool->draw_fini();
    }
    ping->free();
}

//-----------------------------------------------------------------------------

void ogl::irradiance_env::bind(GLenum unit) const
{
//  assert(cube);
//  cube->bind_color(unit);

//  d->bind(unit);
//  L->bind(unit);

    ping->bind_color(unit);
}

//-----------------------------------------------------------------------------
