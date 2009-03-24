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
#include "ogl-pool.hpp"
#include "ogl-frame.hpp"
#include "ogl-process.hpp"

//-----------------------------------------------------------------------------

int ogl::process::count = 0;

ogl::pool *ogl::process::clip_pool = 0;
ogl::node *ogl::process::clip_node = 0;

ogl::pool *ogl::process::cube_pool = 0;
ogl::node *ogl::process::cube_node[6];

//-----------------------------------------------------------------------------

ogl::process::process(const std::string& name) : name(name)
{
    if (count++ == 0)
    {
        init_clip();
        init_cube();
    }
}

ogl::process::~process()
{
    if (--count == 0)
    {
        fini_clip();
        fini_cube();
    }
}

//-----------------------------------------------------------------------------

void ogl::process::init_clip()
{
    assert(clip_pool == 0);

    clip_pool = ::glob->new_pool();
    clip_node = new ogl::node;

    clip_node->add_unit(new ogl::unit("util/clip-fill.obj"));
    clip_pool->add_node(clip_node);
}

void ogl::process::fini_clip()
{
    assert(clip_pool != 0);

    ::glob->free_pool(clip_pool);

    clip_pool = 0;
}

//-----------------------------------------------------------------------------

void ogl::process::init_cube()
{
    assert(cube_pool == 0);

    static const char *cube_file[] = {
        "util/cube-face-negative-x.obj",
        "util/cube-face-positive-x.obj",
        "util/cube-face-negative-y.obj",
        "util/cube-face-positive-y.obj",
        "util/cube-face-negative-z.obj",
        "util/cube-face-positive-z.obj"
    };

    if (cube_pool == 0)
    {
        cube_pool = ::glob->new_pool();

        cube_node[0] = new ogl::node;
        cube_node[1] = new ogl::node;
        cube_node[2] = new ogl::node;
        cube_node[3] = new ogl::node;
        cube_node[4] = new ogl::node;
        cube_node[5] = new ogl::node;

        cube_node[0]->add_unit(new ogl::unit(cube_file[0], false));
        cube_node[1]->add_unit(new ogl::unit(cube_file[1], false));
        cube_node[2]->add_unit(new ogl::unit(cube_file[2], false));
        cube_node[3]->add_unit(new ogl::unit(cube_file[3], false));
        cube_node[4]->add_unit(new ogl::unit(cube_file[4], false));
        cube_node[5]->add_unit(new ogl::unit(cube_file[5], false));

        cube_pool->add_node(cube_node[0]);
        cube_pool->add_node(cube_node[1]);
        cube_pool->add_node(cube_node[2]);
        cube_pool->add_node(cube_node[3]);
        cube_pool->add_node(cube_node[4]);
        cube_pool->add_node(cube_node[5]);
    }
}

void ogl::process::fini_cube()
{
    assert(cube_pool != 0);

    ::glob->free_pool(cube_pool);

    cube_pool = 0;
}

void ogl::process::proc_cube(ogl::frame *cube)
{
    static const GLenum target[] = {
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    };

    // Apply the current binding to all faces of the given cube map.

    cube_pool->prep();
    cube_pool->draw_init();
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        {
            glPolygonOffset(0.0, -1.0f);

            for (int k = 0; k < 6; ++k)
            {
                cube->bind(int(target[k]));
                cube_node[k]->draw();
                cube->free();
            }
        }
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    cube_pool->draw_fini();
}

//-----------------------------------------------------------------------------

