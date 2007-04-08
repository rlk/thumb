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

#include <stdexcept>
#include <iostream>
#include <fstream>

#include <sys/stat.h>
#include <fcntl.h>

#include "directory.hpp"
#include "data.hpp"

//-----------------------------------------------------------------------------

app::buffer::buffer() : ptr(0), len(0)
{
}

app::buffer::~buffer()
{
    delete [] ptr;
}

const void *app::buffer::get(size_t *size) const
{
    if (size)
       *size = len;

    return ptr;
}

//=============================================================================
// File system data archive

app::file_buffer::file_buffer(std::string name)
{
    struct stat info;
    int fd;

    // Open the named file and determine its size.

    if ((fd = open(name.c_str(), O_RDONLY)) == -1)
        throw open_error(name);

    if (fstat(fd, &info) != 0)
        throw stat_error(name);

    // Initialize the buffer.

    len = (size_t) info.st_size;
    ptr = new unsigned char[len + 1];

    // Read all data.

    if (read(fd, ptr, len) < (ssize_t) len)
        throw read_error(name);

    ptr[len] = 0;

    close(fd);
}

bool app::file_archive::find(std::string name)
{
    std::string curr = path + "/" + name;

    std::cout << "file_archive find " << curr << std::endl;

    // Determine whether the named file exists within this archive.

    struct stat info;

    if (stat(curr.c_str(), &info) == 0 && (info.st_mode & S_IFMT) == S_IFREG)
        return true;
    else
        return false;
}

app::buffer_p app::file_archive::load(std::string name)
{
    std::string curr = path + "/" + name;

    std::cout << "file_archive load " << curr << std::endl;

    return new file_buffer(curr);
}

void app::file_archive::list(std::string name, strings& dirs, strings& regs)
{
    std::string curr = path + "/" + name;

    std::cout << "file_archive list " << curr << std::endl;

    dir(name, dirs, regs);
}

//-----------------------------------------------------------------------------

app::data::data()
{
    // Create a prioritized list of available archives.

    archives.push_back(new file_archive("data"));
}

app::data::~data()
{
    std::list<archive_p>::iterator i;

    // Delete all archives.

    for (i = archives.begin(); i != archives.end(); ++i)
        delete *i;
}

const void *app::data::load_dat(std::string name, size_t *size)
{
    std::list<archive_p>::iterator i;

    // If the named buffer has not yet been loaded, load it.

    if (buffers.find(name) == buffers.end())
    {
        // Search the list of archives for the first one with the named buffer.

        for (i = archives.begin(); i != archives.end(); ++i)
            if ((*i)->find(name))
            {
                buffers[name] = (*i)->load(name);
                break;
            }
    }

    // Return the named buffer.

    if (buffers.find(name) != buffers.end())
        return buffers[name]->get(size);
    else
        return 0;
}

void app::data::free_dat(std::string name)
{
    // If the named buffer has been loaded, free it.

    if (buffers.find(name) != buffers.end())
    {
        delete buffers[name];
        buffers.erase(name);
    }
}

//-----------------------------------------------------------------------------
/*
app::data::data(std::string p) : path(p)
{
    if (dir_scan(path).length() == 0)
        throw std::runtime_error("Cannot find data directory.");

    dir_close();
}
*/
//-----------------------------------------------------------------------------
/*
std::string app::data::get_absolute(std::string filename)
{
    std::string directory = path;

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
    std::string directory = path;

    // Filenames with the data directory prepended need it removed.

    if (filename.compare(0, directory.length(), directory) == 0)
        return std::string(filename, directory.length());

    // All others pass through by default.

    return filename;
}
*/
//-----------------------------------------------------------------------------
/*
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
*/
//-----------------------------------------------------------------------------
/*
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
*/
//-----------------------------------------------------------------------------

