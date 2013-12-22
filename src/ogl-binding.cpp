//  Copyright (C) 2007-2011 Robert Kooima
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

#include <SDL.h>
#include <cmath>

#include <app-default.hpp>
#include <app-conf.hpp>
#include <app-glob.hpp>
#include <app-file.hpp>
#include <ogl-pool.hpp>
#include <ogl-texture.hpp>
#include <ogl-program.hpp>
#include <ogl-binding.hpp>

//-----------------------------------------------------------------------------

const ogl::program *ogl::binding::init_program(app::node p,
                                               unit_texture& texture)
{
    // Load the program.

    const std::string   file = p.get_s("file");
    const ogl::program *prog = 0;

    if (!file.empty() && (prog = glob->load_program(file)))
    {
        // Load all textures.

        app::node curr;

        for (app::node n = p.find("texture"); n; n = p.next(n, "texture"))
        {
            // Determine the sampler and image file names.

            const std::string sampler = n.get_s("sampler");
            const std::string name    = n.get_s("name");

            const std::string fail = "default-" + sampler + ".png";

            // Determine the texture unit binding for this sampler.

            if (GLenum unit = prog->unit(sampler))
            {
                if (const ogl::texture *T = ::glob->load_texture(name, fail))
                    texture[unit] = T;
            }
        }
    }
    return prog;
}

ogl::binding::binding(std::string name) :
    name(name),
    depth_program(0),
    color_program(0)
{
    std::string path = "material/" + name + ".xml";

    app::file file(path);

    // Load the local material bindings.

    if (app::node p = file.get_root().find("material"))
    {
        // Load the depth-mode bindings.

        if (app::node n = p.find("program", "mode", "depth"))
            depth_program = init_program(n, depth_texture);

        // Load the color-mode bindings.

        if (app::node n = p.find("program", "mode", "color"))
            color_program = init_program(n, color_texture);
    }
}

ogl::binding::~binding()
{
    // Free all textures.

    unit_texture::iterator i;

    for (i = color_texture.begin(); i != color_texture.end(); ++i)
        ::glob->free_texture(i->second);

    for (i = depth_texture.begin(); i != depth_texture.end(); ++i)
        ::glob->free_texture(i->second);

    color_texture.clear();
    depth_texture.clear();

    // Free all programs.

    if (depth_program) glob->free_program(depth_program);
    if (color_program) glob->free_program(color_program);

    depth_program = 0;
    color_program = 0;
}

//-----------------------------------------------------------------------------

// Determine whether two bindings are equivalent in depth rendering mode.

bool ogl::binding::depth_eq(const binding *that) const
{
    // If the pointers are the same then they are trivially equivalent.

    if (that == this)
        return true;

    // If neither depth program discards then they are functionally equivalent.
    // This optimization is critical for high performance shadowing and z-only.

    if (that->depth_program->discards() == false
           && depth_program->discards() == false) return true;

    // If any programs or textures differ then the bindings are not equivalent.

    if (depth_program != that->depth_program) return false;
    if (depth_texture != that->depth_texture) return false;

    return true;
}

// Determine whether two bindings are equivalent in color rendering mode.

bool ogl::binding::color_eq(const binding *that) const
{
    // If the pointers are the same then they are trivially equivalent.

    if (that == this)
        return true;

    // If any programs or textures differ then the bindings are not equivalent.

    if (color_program != that->color_program) return false;
    if (color_texture != that->color_texture) return false;

    return true;
}

// Determine whether the material binding is opaque.

bool ogl::binding::opaque() const
{
    // If the color program discards, then this binding is not opaque.

    if (color_program->discards())
        return false;

    // If there is no color texture to modulate opacity, then it is opaque.

    if (color_texture.empty())
        return true;

    // Otherwise opacity is determined by the lowest-unit-numbered texture.

    return (*color_texture.begin()).second->opaque();
}

//-----------------------------------------------------------------------------

bool ogl::binding::bind(bool c) const
{
    // TODO: Minimize rebindings

    unit_texture::const_iterator ti;

    if (c)
    {
        if (color_program)
        {
            color_program->bind();

            for (ti = color_texture.begin(); ti != color_texture.end(); ++ti)
                ti->second->bind(ti->first);

            return true;
        }
    }
    else
    {
        if (depth_program)
        {
            depth_program->bind();

            for (ti = depth_texture.begin(); ti != depth_texture.end(); ++ti)
                ti->second->bind(ti->first);

            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
