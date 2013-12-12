//  Copyright (C) 2005-2011 Robert Kooima
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

#include <stdexcept>
#include <sstream>
#include <cassert>
#include <cstdio>

#include <ogl-reflection-env.hpp>
#include <ogl-irradiance-env.hpp>
#include <ogl-sh-basis.hpp>
#include <ogl-d-omega.hpp>
#include <ogl-shadow.hpp>

#include <ogl-uniform.hpp>
#include <ogl-program.hpp>
#include <ogl-texture.hpp>
#include <ogl-binding.hpp>
#include <ogl-surface.hpp>

#include <ogl-image.hpp>
#include <ogl-frame.hpp>
#include <ogl-pool.hpp>

#include <app-glob.hpp>

// TODO: Template some of this repetition?

// TODO: Eliminate the duplicated free_ functions.

// NOTE: The map and set erase(key_type) functions don't seem to work
// correctly under OSX.  For this reason, all map and set removals are
// performed using a found iterator.

//=============================================================================

void app::glob::dump()
{
    // Print the number of cached objects, with names, if possible. This
    // helps track down resource leaks.

    if (size_t qc =  pool_set.size()) printf("%3zu pools\n",  qc);
    if (size_t ic = image_set.size()) printf("%3zu images\n", ic);
    if (size_t fc = frame_set.size()) printf("%3zu frames\n", fc);

    std::map<std::string, surface>::iterator si;
    std::map<std::string, binding>::iterator bi;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, program>::iterator pi;
    std::map<std::string, uniform>::iterator ui;

    if (size_t sc = surface_map.size())
    {
        printf("%3zu surfaces\n", sc);
        for (si = surface_map.begin(); si != surface_map.end(); ++si)
            printf("    %s\n", si->second.ptr->get_name().c_str());
    }

    if (size_t bc = binding_map.size())
    {
        printf("%3zu bindings\n", bc);
        for (bi = binding_map.begin(); bi != binding_map.end(); ++bi)
            printf("    %s\n", bi->second.ptr->get_name().c_str());
    }

    if (size_t tc = texture_map.size())
    {
        printf("%3zu textures\n", tc);
        for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
            printf("    %s\n", ti->second.ptr->get_name().c_str());
    }

    if (size_t pc = program_map.size())
    {
        printf("%3zu programs\n", pc);
        for (pi = program_map.begin(); pi != program_map.end(); ++pi)
            printf("    %s\n", pi->second.ptr->get_name().c_str());
    }

    if (size_t uc = uniform_map.size())
    {
        printf("%3zu uniforms\n", uc);
        for (ui = uniform_map.begin(); ui != uniform_map.end(); ++ui)
            printf("    %s\n", ui->second.ptr->get_name().c_str());
    }
}

app::glob::~glob()
{
    // If all GLOB users properly release their objects then nothing will
    // remain at this point. Deletion here is particularly difficult due
    // to circular dependencies among objects. For these reasons, we take
    // a hard stance on GLOB cleanup. Assert that it is already done.

    dump();

//  assert( pool_set.empty());
//  assert(image_set.empty());
//  assert(frame_set.empty());
//  assert(surface_map.empty());
//  assert(binding_map.empty());
//  assert(texture_map.empty());
//  assert(program_map.empty());
//  assert(uniform_map.empty());
}

//-----------------------------------------------------------------------------

ogl::uniform *app::glob::load_uniform(const std::string& name, GLsizei size)
{
    if (uniform_map.find(name) == uniform_map.end())
    {
        try
        {
            if (ogl::uniform *p = new ogl::uniform(name, size))
            {
                uniform_map[name].ptr = p;
                uniform_map[name].ref = 1;
            }
        }
        catch (std::runtime_error& e)
        {
            return 0;
        }
    }
    else   uniform_map[name].ref++;

    return uniform_map[name].ptr;
}

ogl::uniform *app::glob::dupe_uniform(ogl::uniform *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (uniform_map.find(name) != uniform_map.end())
        {
            uniform_map[name].ref++;
            return uniform_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_uniform(const std::string& name)
{
    std::map<std::string, uniform>::iterator i;

    if ((i = uniform_map.find(name)) != uniform_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            uniform_map.erase(i);
        }
    }
}

void app::glob::free_uniform(ogl::uniform *p)
{
    if (p) free_uniform(p->get_name());
}

//-----------------------------------------------------------------------------

const ogl::program *app::glob::load_program(const std::string& name)
{
    if (program_map.find(name) == program_map.end())
    {
        try
        {
            if (ogl::program *p = new ogl::program(name))
            {
                program_map[name].ptr = p;
                program_map[name].ref = 1;
            }
        }
        catch (std::runtime_error& e)
        {
            fprintf(stderr, "Failed to load program %s\n", name.c_str());
            return 0;
        }
    }
    else   program_map[name].ref++;

    return program_map[name].ptr;
}

const ogl::program *app::glob::dupe_program(const ogl::program *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (program_map.find(name) != program_map.end())
        {
            program_map[name].ref++;
            return program_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_program(const std::string& name)
{
    std::map<std::string, program>::iterator i;

    if ((i = program_map.find(name)) != program_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            program_map.erase(i);
        }
    }
}

void app::glob::free_program(const ogl::program *p)
{
    if (p) free_program(p->get_name());
}

//-----------------------------------------------------------------------------

const ogl::process *app::glob::load_process(const std::string& name, int i)
{
    // Mangle the process name and the argument.

    std::ostringstream str;

    str << name << i;

    if (process_map.find(str.str()) == process_map.end())
    {
        ogl::process *ptr = 0;

        if       (name == "d_omega")
            ptr = new ogl::d_omega       (str.str());
        else if  (name == "shadow")
            ptr = new ogl::shadow        (str.str(), i);
        else if  (name == "sh_basis")
            ptr = new ogl::sh_basis      (str.str(), i);
        else if  (name == "reflection_env")
            ptr = new ogl::reflection_env(str.str(), i);
        else if  (name == "irradiance_env")
            ptr = new ogl::irradiance_env(str.str(), i);

        if (ptr)
        {
            process_map[str.str()].ptr = ptr;
            process_map[str.str()].ref = 1;
        }
    }
    else   process_map[str.str()].ref++;

    return process_map[str.str()].ptr;
}

const ogl::process *app::glob::dupe_process(const ogl::process *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (process_map.find(name) != process_map.end())
        {
            process_map[name].ref++;
            return process_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_process(const std::string& name)
{
    std::map<std::string, process>::iterator i;

    if ((i = process_map.find(name)) != process_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            process_map.erase(i);
        }
    }
}

void app::glob::free_process(const ogl::process *p)
{
    if (p) free_process(p->get_name());
}

//-----------------------------------------------------------------------------

const ogl::texture *app::glob::load_texture(const std::string& name,
                                            const std::string& fallback)
{
    if (texture_map.find(name) == texture_map.end())
    {
        try
        {
            if (ogl::texture *p = new ogl::texture(name))
            {
                texture_map[name].ptr = p;
                texture_map[name].ref = 1;
            }
        }
        catch (std::runtime_error& e)
        {
            if (name == fallback)
                return 0;
            else
                return load_texture(fallback, fallback);
        }
    }
    else   texture_map[name].ref++;

    return texture_map[name].ptr;
}

const ogl::texture *app::glob::dupe_texture(const ogl::texture *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (texture_map.find(name) != texture_map.end())
        {
            texture_map[name].ref++;
            return texture_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_texture(const std::string& name)
{
    std::map<std::string, texture>::iterator i;

    if ((i = texture_map.find(name)) != texture_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            texture_map.erase(i);
        }
    }
}

void app::glob::free_texture(const ogl::texture *p)
{
    if (p) free_texture(p->get_name());
}

//-----------------------------------------------------------------------------

const ogl::binding *app::glob::load_binding(const std::string& name,
                                            const std::string& fallback)
{
    if (binding_map.find(name) == binding_map.end())
    {
        try
        {
            if (ogl::binding *p = new ogl::binding(name))
            {
                binding_map[name].ptr = p;
                binding_map[name].ref = 1;
            }
        }
        catch (std::runtime_error& e)
        {
            if (name == fallback)
                return 0;
            else
                return load_binding(fallback, fallback);
        }
    }
    else   binding_map[name].ref++;

    return binding_map[name].ptr;
}

const ogl::binding *app::glob::dupe_binding(const ogl::binding *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (binding_map.find(name) != binding_map.end())
        {
            binding_map[name].ref++;
            return binding_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_binding(const std::string& name)
{
    std::map<std::string, binding>::iterator i;

    if ((i = binding_map.find(name)) != binding_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            binding_map.erase(i);
        }
    }
}

void app::glob::free_binding(const ogl::binding *p)
{
    if (p) free_binding(p->get_name());
}

//-----------------------------------------------------------------------------

const ogl::surface *app::glob::load_surface(const std::string& name, bool cent)
{
    if (surface_map.find(name) == surface_map.end())
    {
        if (ogl::surface *p = new ogl::surface(name, cent))
        {
            surface_map[name].ptr = p;
            surface_map[name].ref = 1;
        }
    }
    else   surface_map[name].ref++;

    return surface_map[name].ptr;
}

const ogl::surface *app::glob::dupe_surface(const ogl::surface *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (surface_map.find(name) != surface_map.end())
        {
            surface_map[name].ref++;
            return surface_map[name].ptr;
        }
    }
    return 0;
}

void app::glob::free_surface(const std::string& name)
{
    std::map<std::string, surface>::iterator i;

    if ((i = surface_map.find(name)) != surface_map.end())
    {
        if (--i->second.ref == 0)
        {
            delete i->second.ptr;
            surface_map.erase(i);
        }
    }
}

void app::glob::free_surface(const ogl::surface *p)
{
    if (p) free_surface(p->get_name());
}

//-----------------------------------------------------------------------------

ogl::pool *app::glob::new_pool()
{
    ogl::pool *p = new ogl::pool();

    pool_set.insert(p);

    return p;
}

void app::glob::free_pool(ogl::pool *p)
{
    std::set<ogl::pool *>::iterator i;

    if ((i = pool_set.find(p)) != pool_set.end())
    {
        pool_set.erase(i);
        delete p;
    }
}

//-----------------------------------------------------------------------------

ogl::image *app::glob::new_image(GLsizei w, GLsizei h, GLenum T,
                                 GLenum fi, GLenum fe, GLenum t)
{
    ogl::image *p = new ogl::image(w, h, T, fi, fe, t);

    image_set.insert(p);

    return p;
}

void app::glob::free_image(ogl::image *p)
{
    std::set<ogl::image *>::iterator i;

    if ((i = image_set.find(p)) != image_set.end())
    {
        image_set.erase(i);
        delete p;
    }
}

//-----------------------------------------------------------------------------

ogl::frame *app::glob::new_frame(GLsizei w, GLsizei h,
                                 GLenum  t, GLenum  f,
                                 bool c, bool d, bool s)
{
    ogl::frame *p = new ogl::frame(w, h, t, f, c, d, s);

    frame_set.insert(p);

    return p;
}

void app::glob::free_frame(ogl::frame *p)
{
    std::set<ogl::frame *>::iterator i;

    if ((i = frame_set.find(p)) != frame_set.end())
    {
        frame_set.erase(i);
        delete p;
    }
}

//-----------------------------------------------------------------------------

void app::glob::prep()
{
    // Render pre-pass all OpenGL state.

    std::map<std::string, program>::iterator pi;

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        if (pi->second.ptr)
            pi->second.ptr->prep();
}

void app::glob::init()
{
    // Reacquire all OpenGL state.

    std::set<ogl::frame *>::iterator fi;
    std::set<ogl::image *>::iterator ii;
    std::set<ogl::pool  *>::iterator qi;

    for (fi = frame_set.begin(); fi != frame_set.end(); ++fi)
        (*fi)->init();

    for (ii = image_set.begin(); ii != image_set.end(); ++ii)
        (*ii)->init();

    for (qi =  pool_set.begin(); qi !=  pool_set.end(); ++qi)
        (*qi)->init();

    std::map<std::string, program>::iterator pi;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, process>::iterator Pi;

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        if (pi->second.ptr)
            pi->second.ptr->init();

    for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
        if (ti->second.ptr)
            ti->second.ptr->init();

    for (Pi = process_map.begin(); Pi != process_map.end(); ++Pi)
        if (Pi->second.ptr)
            Pi->second.ptr->init();
}

void app::glob::fini()
{
    // Null all texture bindings.

    ogl::free_texture();

    // Release all OpenGL state.

    std::map<std::string, process>::iterator Pi;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, program>::iterator pi;

    for (Pi = process_map.begin(); Pi != process_map.end(); ++Pi)
        if (Pi->second.ptr)
            Pi->second.ptr->fini();

    for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
        if (ti->second.ptr)
            ti->second.ptr->fini();

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        if (pi->second.ptr)
            pi->second.ptr->fini();

    std::set<ogl::pool  *>::iterator qi;
    std::set<ogl::image *>::iterator ii;
    std::set<ogl::frame *>::iterator fi;

    for (qi =  pool_set.begin(); qi !=  pool_set.end(); ++qi)
        (*qi)->fini();

    for (ii = image_set.begin(); ii != image_set.end(); ++ii)
        (*ii)->fini();

    for (fi = frame_set.begin(); fi != frame_set.end(); ++fi)
        (*fi)->fini();
}

//-----------------------------------------------------------------------------

