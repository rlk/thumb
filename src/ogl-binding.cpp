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

#include <SDL.h>
#include <cmath>

#include "default.hpp"
#include "app-conf.hpp"
#include "app-glob.hpp"
#include "app-serial.hpp"
#include "ogl-pool.hpp"
#include "ogl-texture.hpp"
#include "ogl-process.hpp"
#include "ogl-program.hpp"
#include "ogl-binding.hpp"

//-----------------------------------------------------------------------------

const ogl::program *ogl::binding::init_program(app::node node,
                                               unit_texture& texture,
                                               unit_process& process)
{
    // Load the program.

    const std::string   file = app::get_attr_s(node, "file");
    const ogl::program *prog = 0;

    if (!file.empty() && (prog = glob->load_program(file)))
    {
        // Load all textures.

        app::node curr;

        for (curr = app::find(node,       "texture"); curr;
             curr = app::next(node, curr, "texture"))
        {
            // Determine the sampler and image file names.

            const std::string sampler = app::get_attr_s(curr, "sampler");
            const std::string name    = app::get_attr_s(curr, "name");

            const std::string fail = "default-" + sampler + ".png";

            // Determine the texture unit binding for this sampler.

            if (GLenum unit = prog->unit(sampler))
            {
                if (const ogl::texture *T = ::glob->load_texture(name, fail))
                    texture[unit] = T;
            }
        }

        // Load all processes.

        for (curr = app::find(node,       "process"); curr;
             curr = app::next(node, curr, "process"))
        {
            // Determine the sampler and image file names.

            const std::string sampler = app::get_attr_s(curr, "sampler");
            const std::string name    = app::get_attr_s(curr, "name");

            // Determine the texture unit binding for this sampler.

            if (GLenum unit = prog->unit(sampler))
            {
                if (const ogl::process *P = ::glob->load_process(name))
                    process[unit] = P;
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

    app::serial file(path.c_str());
    app::node   root;
    app::node   node;

    // Load the local material bindings.

    if ((root = app::find(file.get_head(), "material")))
    {
        // Load the depth-mode bindings.

        if ((node = app::find(root, "program", "mode", "depth")))
            depth_program = init_program(node, depth_texture,
                                               depth_process);

        // Load the color-mode bindings.

        if ((node = app::find(root, "program", "mode", "color")))
            color_program = init_program(node, color_texture,
                                               color_process);
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

    // Free all processes.
    
    unit_process::iterator j;

    for (j = color_process.begin(); j != color_process.end(); ++j)
        ::glob->free_process(j->second);

    for (j = depth_process.begin(); j != depth_process.end(); ++j)
        ::glob->free_process(j->second);

    color_process.clear();
    depth_process.clear();

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

    // If both bindings are opaque then they are functionally equivalent.

    if (opaque() && that->opaque())
        return true;

    // If any programs, textures, or processes differ, then the bindings
    // are not equivalent.

    if (depth_program != that->depth_program) return false;
    if (depth_texture != that->depth_texture) return false;
    if (depth_process != that->depth_process) return false;

    return true;
}

// Determine whether two bindings are equivalent in color rendering mode.

bool ogl::binding::color_eq(const binding *that) const
{
    // If the pointers are the same then they are trivially equivalent.

    if (that == this)
        return true;

    // If any programs, textures, or processes differ, then the bindings
    // are not equivalent.

    if (color_program != that->color_program) return false;
    if (color_texture != that->color_texture) return false;
    if (color_process != that->color_process) return false;

    return true;
}

// Determine whether the material binding is opaque.

bool ogl::binding::opaque() const
{
    // HACK (borderline): Test the lowest-unit-numbered texture for opacity.

    if (!color_texture.empty())
        return (*color_texture.begin()).second->opaque();

    return true;
}

//-----------------------------------------------------------------------------

bool ogl::binding::bind(bool c) const
{
    // TODO: Minimize rebindings

    unit_texture::const_iterator ti;
    unit_process::const_iterator pi;

    if (c)
    {
        if (color_program)
        {
            color_program->bind();

            // Bind all textures

            for (ti = color_texture.begin(); ti != color_texture.end(); ++ti)
                ti->second->bind(ti->first);

            for (pi = color_process.begin(); pi != color_process.end(); ++pi)
                pi->second->bind(pi->first);

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
            
            for (pi = depth_process.begin(); pi != depth_process.end(); ++pi)
                pi->second->bind(pi->first);
            
            return true;
        }
    }
    return true; // HACK HACK (Convert ALL objects to use material bindings.)
}

//-----------------------------------------------------------------------------
