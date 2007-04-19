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

#include "data.hpp"
#include "util.hpp"
#include "dir.hpp"

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

bool app::file_archive::find(std::string name) const
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

app::buffer_p app::file_archive::load(std::string name) const
{
    std::string curr = path + "/" + name;

    std::cout << "file_archive load " << curr << std::endl;

    return new file_buffer(curr);
}

void app::file_archive::list(std::string name, strset& dirs,
                                               strset& regs) const
{
    std::string curr = path + "/" + name;

    std::cout << "file_archive list " << curr << std::endl;

    dir(curr, dirs, regs);
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

const void *app::data::load(std::string name, size_t *size)
{
    // If the named buffer has not yet been loaded, load it.

    if (buffers.find(name) == buffers.end())
    {
        // Search the list of archives for the first one with the named buffer.

        for (archive_c i = archives.begin(); i != archives.end(); ++i)
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

void app::data::list(std::string name, strset& dirs, strset& regs)
{
    // Merge the file lists of all archives.

    for (archive_c i = archives.begin(); i != archives.end(); ++i)
        (*i)->list(name, dirs, regs);
}

void app::data::free(std::string name)
{
    // If the named buffer has been loaded, free it.

    if (buffers.find(name) != buffers.end())
    {
        delete buffers[name];
        buffers.erase(name);
    }
}

//-----------------------------------------------------------------------------
