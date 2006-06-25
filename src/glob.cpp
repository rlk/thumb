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

#include "main.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

app::glob::~glob()
{
    std::map<std::string, ogl::shader *>::iterator i;
    std::map<std::string, ogl::image  *>::iterator j;

    for (i = shader_map.begin(); i != shader_map.end(); ++i)
        delete i->second;

    for (j = image_map.begin(); j != image_map.end(); ++j)
        delete j->second;
}

//-----------------------------------------------------------------------------

const ogl::shader *app::glob::get_shader(std::string name)
{
    if (shader_map[name] == 0)
        shader_map[name] = new ogl::shader(::data->get_txt(name + ".vert"),
                                           ::data->get_txt(name + ".frag"));
    return shader_map[name];
}

const ogl::image *app::glob::get_image(std::string name)
{
    if (image_map[name] == 0)
        image_map[name] = new ogl::image_file(name);

    return image_map[name];
}

//-----------------------------------------------------------------------------

