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

#include "ogl-binding.hpp"
#include "app-serial.hpp"
#include "app-glob.hpp"

//-----------------------------------------------------------------------------

GLfloat ogl::binding::split[4];

//-----------------------------------------------------------------------------

ogl::binding::binding(std::string name) :
    name(name),
    depth_program(0),
    color_program(0)
{
    std::string path = "material/" + name + ".xml";

    app::serial file(path.c_str());
    app::node   root;
    app::node   node;

    if ((root = app::find(file.get_head(), "material")))
    {
        // Load the color-mode program.

        if ((node = app::find(root, "program", "mode", "color")))
            color_program = glob->load_program(app::get_attr_s(node, "file"));

        // Load the depth-mode program.

        if ((node = app::find(root, "program", "mode", "depth")))
            depth_program = glob->load_program(app::get_attr_s(node, "file"));

        // Load all textures.

        for (node = app::find(root,       "texture"); node;
             node = app::next(root, node, "texture"))
        {
            std::string name(app::get_attr_s(node, "name"));
            std::string file(app::get_attr_s(node, "file"));

            std::string path = "texture/" + file;

            // Determine the texture unit binding for each texture.

            GLenum depth_unit = depth_program ? depth_program->unit(name) : 0;
            GLenum color_unit = color_program ? color_program->unit(name) : 0;

            // Load the texture (with reference count +2).

            if (depth_unit)
                depth_texture[depth_unit] = ::glob->load_texture(path);
            if (color_unit)
                color_texture[color_unit] = ::glob->load_texture(path);
        }
    }
    init();
}

ogl::binding::~binding()
{
    // Free all textures.

    unit_map::iterator i;

    for (i = color_texture.begin(); i != color_texture.end(); ++i)
        ::glob->free_texture(i->second);

    for (i = depth_texture.begin(); i != depth_texture.end(); ++i)
        ::glob->free_texture(i->second);

    color_texture.clear();
    depth_texture.clear();

    // Free all programs.

    if (depth_program) glob->free_program(depth_program);
    if (color_program) glob->free_program(color_program);
}

//-----------------------------------------------------------------------------

void ogl::binding::set_split(GLfloat a, GLfloat b, GLfloat c, GLfloat d)
{
    split[0] = a;
    split[1] = b;
    split[2] = c;
    split[3] = d; 
}

//-----------------------------------------------------------------------------

bool ogl::binding::depth_eq(const binding *that) const
{
    return (depth_program == that->depth_program);
}

bool ogl::binding::color_eq(const binding *that) const
{
    return (color_program == that->color_program);
}

//-----------------------------------------------------------------------------

void ogl::binding::bind(bool c) const
{
    // TODO: Minimize rebindings

    unit_map::const_iterator i;

    if (c)
    {
        if (color_program)
            color_program->bind();

        for (i = color_texture.begin(); i != color_texture.end(); ++i)
            i->second->bind(i->first);
    }
    else
    {
        if (depth_program)
            depth_program->bind();

        for (i = color_texture.begin(); i != color_texture.end(); ++i)
            i->second->bind(i->first);
    }
}

//-----------------------------------------------------------------------------

void ogl::binding::init()
{
}

void ogl::binding::fini()
{
}

//-----------------------------------------------------------------------------
