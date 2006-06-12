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
#include <fstream>

#include "data.hpp"
#include "obj.h"

//-----------------------------------------------------------------------------

std::string app::data::get_absolute(std::string filename)
{
    std::string directory = path + "/";

    // Absolute filenames pass through unchanged.

    if (filename[0] == '/')
        return filename;

    // Filenames with the data directory already prepended also pass unchanged.

    if (filename.compare(0, directory.length(), directory) == 0)
        return filename;

    // Relative filenames need the data directory prepended.

    return directory + filename;
}

std::string app::data::get_relative(std::string filename)
{
    std::string directory = path + "/";

    // Filenames with the data directory prepended need it removed.

    if (filename.compare(0, directory.length(), directory) == 0)
        return std::string(filename, directory.length());

    // All others pass through by default.

    return filename;
}

//-----------------------------------------------------------------------------

int app::data::get_obj(std::string filename)
{
    std::string pathname = get_absolute(filename);

    // Load the named OBJ only if it is not already loaded.

    if (obj_map.find(filename) == obj_map.end())
        obj_map[filename] = obj_add_file(pathname.c_str());

    return obj_map[filename];
}

std::string app::data::get_txt(std::string filename)
{
    std::string pathname = get_absolute(filename);

    // Load the named text file only if it is not already loaded.

    if (txt_map.find(filename) == txt_map.end())
    {
        std::ifstream file(pathname.c_str());
        std::string   line;

        while (std::getline(file, line))
            txt_map[filename] += line + "\n";
    }

    return txt_map[filename];
}

void *app::data::get_img(std::string filename, int& w, int& h, int& b)
{
    std::string pathname = get_absolute(filename);
    img I;

    // Load the named image only if it is not already loaded.

    if (img_map.find(filename) == img_map.end())
    {
        I.p = obj_read_image(pathname.c_str(), &I.w, &I.h, &I.b);
        img_map[filename] = I;
    }

    w    = img_map[filename].w;
    h    = img_map[filename].h;
    b    = img_map[filename].b;
    return img_map[filename].p;
}

//-----------------------------------------------------------------------------

app::data::~data()
{
    std::map<std::string, int>::iterator i;
    std::map<std::string, img>::iterator j;

    // Delete all OBJ structures.

    obj_reset_all();

    // Release all image buffers.

    for (j = img_map.begin(); j != img_map.end(); ++j)
        free(j->second.p);
}

//-----------------------------------------------------------------------------

