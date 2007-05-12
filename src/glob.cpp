//  Copyright (C) 2005 Robert Kooima
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

#include <iostream>

#include "glob.hpp"

//=============================================================================

app::glob::~glob()
{
    // Release all storage and OpenGL state.

    std::map<std::string, surface>::iterator si;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, program>::iterator pi;

    for (si = surface_map.begin(); si != surface_map.end(); ++si)
        delete si->second.ptr;

    for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
        delete ti->second.ptr;

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        delete pi->second.ptr;

    std::set<ogl::image *>::iterator ii;
    std::set<ogl::frame *>::iterator fi;

    for (ii = image_set.begin(); ii != image_set.end(); ++ii)
        delete (*ii);

    for (fi = frame_set.begin(); fi != frame_set.end(); ++fi)
        delete (*fi);
}

//-----------------------------------------------------------------------------

const ogl::program *app::glob::load_program(std::string vert,
                                            std::string frag)
{
    std::string name = vert + frag;

    if (program_map.find(name) == program_map.end())
    {
        program_map[name].ptr = new ogl::program(vert, frag);
        program_map[name].ref = 1;
    }
    else   program_map[name].ref++;

    return program_map[name].ptr;
}

void app::glob::free_program(std::string vert,
                             std::string frag)
{
    std::string name = vert + frag;

    if (program_map.find(name) != program_map.end())
    {
        if (--program_map[name].ref == 0)
        {
            delete program_map[name].ptr;
            program_map.erase(name);
        }
    }
}

void app::glob::free_program(const ogl::program *p)
{
    if (p) free_program(p->get_vert_name(),
                        p->get_frag_name());
}

void app::glob::dupe_program(const ogl::program *p)
{
    if (p)
    {
        const std::string& vert = p->get_vert_name();
        const std::string& frag = p->get_frag_name();

        std::string name = vert + frag;

        if (program_map.find(name) != program_map.end())
            program_map[name].ref++;
    }
}

//-----------------------------------------------------------------------------

const ogl::texture *app::glob::load_texture(std::string name)
{
    if (texture_map.find(name) == texture_map.end())
    {
        texture_map[name].ptr = new ogl::texture(name);
        texture_map[name].ref = 1;
    }
    else   texture_map[name].ref++;

    return texture_map[name].ptr;
}

void app::glob::free_texture(std::string name)
{
    if (texture_map.find(name) != texture_map.end())
    {
        if (--texture_map[name].ref == 0)
        {
            delete texture_map[name].ptr;
            texture_map.erase(name);
        }
    }
}

void app::glob::free_texture(const ogl::texture *p)
{
    if (p) free_texture(p->get_name());
}

void app::glob::dupe_texture(const ogl::texture *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (texture_map.find(name) != texture_map.end())
            texture_map[name].ref++;
    }
}

//-----------------------------------------------------------------------------

const ogl::surface *app::glob::load_surface(std::string name)
{
    if (surface_map.find(name) == surface_map.end())
    {
        surface_map[name].ptr = new ogl::surface(name);
        surface_map[name].ref = 1;

        fini_geometry();
        init_geometry();
    }
    else   surface_map[name].ref++;

    return surface_map[name].ptr;
}

void app::glob::free_surface(std::string name)
{
    if (surface_map.find(name) != surface_map.end())
    {
        if (--surface_map[name].ref == 0)
        {
            delete surface_map[name].ptr;
            surface_map.erase(name);
        }
    }
}

void app::glob::free_surface(const ogl::surface *p)
{
    if (p) free_surface(p->get_name());
}

void app::glob::dupe_surface(const ogl::surface *p)
{
    if (p)
    {
        const std::string& name = p->get_name();

        if (surface_map.find(name) != surface_map.end())
            surface_map[name].ref++;
    }
}

//-----------------------------------------------------------------------------

ogl::image *app::glob::new_image(GLsizei w, GLsizei h, GLenum t, GLenum f)
{
    ogl::image *p = new ogl::image(w, h, t, f);

    image_set.insert(p);

    return p;
}

void app::glob::free_image(ogl::image *p)
{
    image_set.erase(p);

    delete p;
}

//-----------------------------------------------------------------------------

ogl::frame *app::glob::new_frame(GLsizei w, GLsizei h, GLenum t,
                                                       GLenum c,
                                                       GLenum d)
{
    ogl::frame *p = new ogl::frame(w, h, t, c, d);

    frame_set.insert(p);

    return p;
}

void app::glob::free_frame(ogl::frame *p)
{
    frame_set.erase(p);

    delete p;
}

//-----------------------------------------------------------------------------

#define OFFSET(i) ((char *) (i))

void app::glob::bind_geometry()
{
/*
    GLsizei s = sizeof (ogl::vert);

    glEnableVertexAttribArrayARB(6);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    glTexCoordPointer       (   2, GL_FLOAT,    s, OFFSET(36));
    glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, OFFSET(24));
    glNormalPointer         (      GL_FLOAT,    s, OFFSET(12));
    glVertexPointer         (   3, GL_FLOAT,    s, OFFSET( 0));

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
*/
}

void app::glob::init_geometry()
{
/*
    std::map<std::string, surface>::iterator si;

    // Sum all vertex array and element array sizes.

    GLsizei vsz = 0;
    GLsizei esz = 0;

    for (si = surface_map.begin(); si != surface_map.end(); ++si)
    {
        esz += si->second.ptr->esize();
        vsz += si->second.ptr->vsize();
    }

    std::cout << "vbo " << vsz << " ebo " << esz << std::endl;

    // Initialize vertex and element array buffers of that size.

    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, vsz, 0, GL_STATIC_DRAW_ARB);

    glGenBuffersARB(1, &ebo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, esz, 0, GL_STATIC_DRAW_ARB);

    // Copy all vertices to the buffer object.

    vsz = 0;
    esz = 0;

    for (si = surface_map.begin(); si != surface_map.end(); ++si)
    {
        esz = si->second.ptr->ecopy(esz, vsz);
        vsz = si->second.ptr->vcopy(vsz);
    }

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
*/
}

void app::glob::fini_geometry()
{
/*
    if (ebo) glDeleteBuffersARB(1, &ebo);
    if (vbo) glDeleteBuffersARB(1, &vbo);

    ebo = 0;
    vbo = 0;
*/
}

//-----------------------------------------------------------------------------

void app::glob::init()
{
    // Reacquire all OpenGL state.

    std::map<std::string, program>::iterator pi;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, surface>::iterator si;

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        pi->second.ptr->init();

    for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
        ti->second.ptr->init();

    for (si = surface_map.begin(); si != surface_map.end(); ++si)
        si->second.ptr->init();

    std::set<ogl::image *>::iterator ii;
    std::set<ogl::frame *>::iterator fi;

    for (ii = image_set.begin(); ii != image_set.end(); ++ii)
        (*ii)->init();

    for (fi = frame_set.begin(); fi != frame_set.end(); ++fi)
        (*fi)->init();

    init_geometry();

}

void app::glob::fini()
{
    // Release all OpenGL state.

    fini_geometry();

    std::set<ogl::frame *>::iterator fi;
    std::set<ogl::image *>::iterator ii;

    for (fi = frame_set.begin(); fi != frame_set.end(); ++fi)
        (*fi)->fini();

    for (ii = image_set.begin(); ii != image_set.end(); ++ii)
        (*ii)->fini();

    std::map<std::string, surface>::iterator si;
    std::map<std::string, texture>::iterator ti;
    std::map<std::string, program>::iterator pi;

    for (si = surface_map.begin(); si != surface_map.end(); ++si)
        si->second.ptr->fini();

    for (ti = texture_map.begin(); ti != texture_map.end(); ++ti)
        ti->second.ptr->fini();

    for (pi = program_map.begin(); pi != program_map.end(); ++pi)
        pi->second.ptr->fini();
}

//-----------------------------------------------------------------------------

