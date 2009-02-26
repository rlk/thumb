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
#include "ogl-frame.hpp"
#include "ogl-texture.hpp"
#include "ogl-program.hpp"
#include "ogl-binding.hpp"

//-----------------------------------------------------------------------------

std::vector<ogl::frame *> ogl::binding::shadow;

ogl::frame *ogl::binding::diff_irradiance = 0;
ogl::frame *ogl::binding::spec_irradiance = 0;

//-----------------------------------------------------------------------------

bool ogl::binding::init_shadow()
{
    // Initialize the shadow maps if necessary.

    if (shadow.empty())
    {
        if (ogl::do_shadows)
        {
            int n = ::conf->get_i("shadow_map_splits");
            int s = ::conf->get_i("shadow_map_resolution");

            shadow.reserve(n);

            for (int i = 0; i < n; ++i)
                shadow.push_back(::glob->new_frame(s, s,
                                                   GL_TEXTURE_2D,
                                                   GL_RGBA8,
                                                   false, true, false));
        }
    }
    return !shadow.empty();
}

int ogl::binding::shadow_count()
{
    return shadow.size();
}

bool ogl::binding::bind_shadow(int i)
{
    if (i < int(shadow.size()) && shadow[i])
    {
        shadow[i]->bind();
        return true;
    }
    return false;
}

void ogl::binding::free_shadow(int i)
{
    if (i < int(shadow.size()) && shadow[i])
    {
        shadow[i]->free();
    }
}

//-----------------------------------------------------------------------------

bool ogl::binding::init_irradiance()
{
    // Initialize the irradiance maps if necessary.

    if (spec_irradiance == 0)
        spec_irradiance = ::glob->new_frame(256, 256, GL_TEXTURE_CUBE_MAP,
                                            GL_RGBA8, true, false, false);

    return (spec_irradiance != 0);
}

bool ogl::binding::bind_irradiance(int target)
{
    if (spec_irradiance)
        spec_irradiance->bind(target);

    return spec_irradiance;
}

void ogl::binding::free_irradiance()
{
    if (spec_irradiance)
        spec_irradiance->free();
}

void ogl::binding::prep_irradiance()
{
}

//-----------------------------------------------------------------------------

const ogl::program *ogl::binding::init_program(app::node node,
                                               unit_texture& texture,
                                               unit_frame&   c_frame,
                                               unit_frame&   d_frame)
{
    const char         *name = 0;
    const ogl::program *prog = 0;

    // Load the program.

    if ((name = app::get_attr_s(node, "file", 0)) &&
        (prog = glob->load_program(std::string(name))))
    {
        // Load all textures.

        app::node curr;

        for (curr = app::find(node,       "texture"); curr;
             curr = app::next(node, curr, "texture"))
        {
            // Determine the sampler name.

            std::string sampler(app::get_attr_s(curr, "sampler"));

            // Determine the texture unit binding for this sampler.

            if (GLenum unit = prog->unit(sampler))
            {
                // Handle an image file texture.

                if (const char *s = app::get_attr_s(curr, "file"))
                {
                    std::string file(s);
                    std::string fail = "default-" + sampler + ".png";

                    texture[unit] = ::glob->load_texture(file, fail);
                }

                // Handle a procedural texture.
                
                if (const char *s = app::get_attr_s(curr, "proc"))
                {
                    std::string proc(s);

                    if (proc == "diff_irradiance")
                        c_frame[unit] = diff_irradiance;
                    if (proc == "spec_irradiance")
                        c_frame[unit] = spec_irradiance;
                    if (proc == "shadow")
                        d_frame[unit] = shadow[app::get_attr_d(curr, "level")];
                }
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

    // Initialize the shared procedural materials, if necessary.

    init_shadow();
    init_irradiance();

    // Load the local material bindings.

    if ((root = app::find(file.get_head(), "material")))
    {
        // Load the depth-mode bindings.

        if ((node = app::find(root, "program", "mode", "depth")))
            depth_program = init_program(node, depth_texture,
                                               depth_c_frame,
                                               depth_d_frame);

        // Load the color-mode bindings.

        if ((node = app::find(root, "program", "mode", "color")))
            color_program = init_program(node, color_texture,
                                               color_c_frame,
                                               color_d_frame);
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

    // TODO: Free all frames. They are statically owned by the bindings.
    // This may be an argument for the process object as a frame abstraction.

/*
    unit_frame::iterator j;

    for (j = color_d_frame.begin(); j != color_d_frame.end(); ++j)
        ::glob->free_frame(j->second);

    for (j = color_c_frame.begin(); j != color_c_frame.end(); ++j)
        ::glob->free_frame(j->second);

    for (j = depth_d_frame.begin(); j != depth_d_frame.end(); ++j)
        ::glob->free_frame(j->second);

    for (j = depth_c_frame.begin(); j != depth_c_frame.end(); ++j)
        ::glob->free_frame(j->second);
*/
    color_d_frame.clear();
    color_c_frame.clear();
    depth_d_frame.clear();
    depth_c_frame.clear();

    // Free all programs.

    if (depth_program) glob->free_program(depth_program);
    if (color_program) glob->free_program(color_program);
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

    // If the programs differ then the bindings are not equivalent.

    if (depth_program != that->depth_program)
        return false;

    // If the textures differ then the bindings are not equivalent.

    if (depth_texture != that->depth_texture)
        return false;

    return true;
}

// Determine whether two bindings are equivalent in color rendering mode.

bool ogl::binding::color_eq(const binding *that) const
{
    // If the pointers are the same then they are trivially equivalent.

    if (that == this)
        return true;

    // If the programs differ then the bindings are not equivalent.

    if (color_program != that->color_program)
        return false;

    // If the textures differ then the bindings are not equivalent.

    if (color_texture != that->color_texture)
        return false;

    return true;
}

// Determine whether the material binding is opaque.

bool ogl::binding::opaque() const
{
    // HACK: test lowest-unit-numbered texture for opacity.

    if (!color_texture.empty())
        return (*color_texture.begin()).second->opaque();

    return true;
}

//-----------------------------------------------------------------------------

bool ogl::binding::bind(bool c) const
{
    // TODO: Minimize rebindings

    unit_texture::const_iterator ti;
    unit_frame  ::const_iterator fi;

    if (c)
    {
        if (color_program)
        {
            color_program->bind();

            // Bind all textures

            for (ti = color_texture.begin(); ti != color_texture.end(); ++ti)
                ti->second->bind(ti->first);

            for (fi = color_c_frame.begin(); fi != color_c_frame.end(); ++fi)
                fi->second->bind_color(fi->first);

            for (fi = color_d_frame.begin(); fi != color_d_frame.end(); ++fi)
                fi->second->bind_depth(fi->first);

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
            
            for (fi = depth_c_frame.begin(); fi != depth_c_frame.end(); ++fi)
                fi->second->bind_depth(fi->first);

            for (fi = depth_d_frame.begin(); fi != depth_d_frame.end(); ++fi)
                fi->second->bind_depth(fi->first);

            return true;
        }
    }
    return true; // HACK HACK (Convert ALL objects to use material bindings.)
}

//-----------------------------------------------------------------------------
