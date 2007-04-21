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

#include "glob.hpp"

//=============================================================================

app::glob::~glob()
{
    fini();
}

//-----------------------------------------------------------------------------

const ogl::program *app::glob::load_program(std::string name)
{
    if (program_map.find(name) == program_map.end())
    {
        program_map[name].ptr = new ogl::program(name);
        program_map[name].ref = 1;
    }
    return program_map[name].ptr;
}

void app::glob::free_program(std::string name)
{
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
    free_program(p->get_name());
}

void app::glob::dupe_program(const ogl::program *p)
{
    const std::string& name = p->get_name();

    if (program_map.find(name) != program_map.end())
        program_map[name].ref++;
}

//-----------------------------------------------------------------------------

const ogl::texture *app::glob::load_texture(std::string name)
{
    if (texture_map.find(name) == texture_map.end())
    {
        texture_map[name].ptr = new ogl::texture(name);
        texture_map[name].ref = 1;
    }
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
    free_texture(p->get_name());
}

void app::glob::dupe_texture(const ogl::texture *p)
{
    const std::string& name = p->get_name();

    if (texture_map.find(name) != texture_map.end())
        texture_map[name].ref++;
}

//-----------------------------------------------------------------------------

const ogl::geodata *app::glob::load_geodata(std::string name)
{
    if (geodata_map.find(name) == geodata_map.end())
    {
        geodata_map[name].ptr = new ogl::geodata(name);
        geodata_map[name].ref = 1;
    }
    return geodata_map[name].ptr;
}

void app::glob::free_geodata(std::string name)
{
    if (geodata_map.find(name) != geodata_map.end())
    {
        if (--geodata_map[name].ref == 0)
        {
            delete geodata_map[name].ptr;
            geodata_map.erase(name);
        }
    }
}

void app::glob::free_geodata(const ogl::geodata *p)
{
    free_geodata(p->get_name());
}

void app::glob::dupe_geodata(const ogl::geodata *p)
{
    const std::string& name = p->get_name();

    if (geodata_map.find(name) != geodata_map.end())
        geodata_map[name].ref++;
}

//-----------------------------------------------------------------------------

ogl::imgdata *app::glob::load_imgdata(GLsizei w, GLsizei h, GLsizei b)
{
    ogl::imgdata *p = new ogl::imgdata(w, h, b);

    imgdata_set.insert(p);

    return p;
}

void app::glob::free_imgdata(ogl::imgdata *p)
{
    imgdata_set.erase(p);

    delete p;
}

//-----------------------------------------------------------------------------

void app::glob::init()
{
    // Reacquire all OpenGL state.

    std::map<std::string, program>::iterator i;
    std::map<std::string, texture>::iterator j;
    std::map<std::string, geodata>::iterator k;
    std::set<ogl::imgdata *>::iterator       l;

    for (i = program_map.begin(); i != program_map.end(); ++i)
        i->second.ptr = new ogl::program(i->first);

    for (j = texture_map.begin(); j != texture_map.end(); ++j)
        j->second.ptr = new ogl::texture(j->first);

    for (k = geodata_map.begin(); k != geodata_map.end(); ++k)
        k->second.ptr = new ogl::geodata(k->first);

    for (l = imgdata_set.begin(); l != imgdata_set.end(); ++l)
        (*l)->init();
}

void app::glob::fini()
{
    // Release all OpenGL state.

    std::map<std::string, program>::iterator i;
    std::map<std::string, texture>::iterator j;
    std::map<std::string, geodata>::iterator k;
    std::set<ogl::imgdata *>::iterator       l;

    for (i = program_map.begin(); i != program_map.end(); ++i)
        delete i->second.ptr;

    for (j = texture_map.begin(); j != texture_map.end(); ++j)
        delete j->second.ptr;

    for (k = geodata_map.begin(); k != geodata_map.end(); ++k)
        delete k->second.ptr;

    for (l = imgdata_set.begin(); l != imgdata_set.end(); ++l)
        (*l)->fini();
}

//-----------------------------------------------------------------------------

