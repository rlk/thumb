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
#include <cstring>
#include <cstdlib>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "data.hpp"
#include "conf.hpp"
#include "util.hpp"
#include "dir.hpp"

//-----------------------------------------------------------------------------

app::buffer::buffer() : ptr(0), len(0)
{
}

app::buffer::~buffer()
{
    if (ptr) delete [] ptr;
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

    // Open the named file for reading and determine its size.

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

    // Null-terminate.  (This will be a problem if we mmap.)

    ptr[len] = 0;

    close(fd);
}

bool app::file_archive::find(std::string name) const
{
    std::string curr = path + "/" + name;

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

    return new file_buffer(curr);
}

bool app::file_archive::save(std::string name,
                             const void *ptr, size_t *len) const
{
    std::string curr = path + "/" + name;

    // Ensure the archive is writable and the directory exists.

    if (writable && mkpath(curr))
    {
        size_t count = len ? (*len) : strlen((const char *) ptr);
        int fd;

        // Open the named file for writing.

        if ((fd = open(curr.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666)) ==-1)
            throw open_error(name);

        // Write all data.

        if (write(fd, ptr, count) < (ssize_t) count)
            throw write_error(name);

        close(fd);

        return true;
    }
    return false;
}

void app::file_archive::list(std::string name, strset& dirs,
                                               strset& regs) const
{
    std::string curr = path + "/" + name;

    dir(curr, dirs, regs);
}

//=============================================================================

void app::data::load()
{
    if (head) mxmlDelete(head);

    head = 0;
    root = 0;

    const char *buff;

    if ((buff = (const char *) load(file)))
    {
        head = mxmlLoadString(NULL, buff, MXML_TEXT_CALLBACK);
        root = mxmlFindElement(head, head, "data", 0, 0, MXML_DESCEND_FIRST);
    }

    free(file);
}

std::string app::data::translate(std::string& file) const
{
    // Try to map the named file onto a configured alternative.

    if (::conf && root)
    {
        mxml_node_t *node;
        mxml_node_t *curr;

        // Find file elements with the given name.

        for (node = mxmlFindElement(root, root, "file", "name", file.c_str(),
                                    MXML_DESCEND_FIRST);
             node;
             node = mxmlFindElement(node, root, "file", "name", file.c_str(),
                                    MXML_NO_DESCEND))
        {
            // Find an option element matching the current config.

            for (curr = mxmlFindElement(node, node, "option",
                                        NULL, NULL, MXML_DESCEND_FIRST);
                 curr;
                 curr = mxmlFindElement(curr, node, "option",
                                        NULL, NULL, MXML_NO_DESCEND))
            {
                std::string type (mxmlElementGetAttr(curr, "type"));
                std::string name (mxmlElementGetAttr(curr, "name"));
                std::string value(mxmlElementGetAttr(curr, "value"));

                // If the option matches the setting, return the mapping.

                if (type == "string")
                {
                    if (::conf->get_s(name) == value)
                        return std::string(curr->child->value.text.string);
                }
                if (type == "int")
                {
                    if (::conf->get_i(name) == atoi(value.c_str()))
                        return std::string(curr->child->value.text.string);
                }
            }
        }
    }

    // No mapping was found.  Return the original string.

    return file;
}

//-----------------------------------------------------------------------------

app::data::data(std::string file) : file(file), head(0), root(0)
{
    // Use a writable archive in the user's home directory.

    char *home;

    if ((home = getenv("HOME")))
    {
        std::string name = std::string(home) + "/.thumb";
        archives.push_back(new file_archive(name, true));
    }

    // Use read-only archive in the current working directory.

    archives.push_back(new file_archive("data", false));

    // Load the data mapping configuration file.

    load();
}

app::data::~data()
{
    std::list<archive_p>::iterator i;

    // Delete all archives.

    for (i = archives.begin(); i != archives.end(); ++i)
        delete *i;

    // Delete the configuration.

    if (head) mxmlDelete(head);
}

const void *app::data::load(std::string name, size_t *len)
{
    // If the named buffer has not yet been loaded, load it.

    if (buffers.find(name) == buffers.end())
    {
        // Search the list of archives for the first one with the named buffer.

        for (archive_c i = archives.begin(); i != archives.end(); ++i)
        {
            std::string rename = translate(name);

            if ((*i)->find(rename))
            {
                buffers[name] = (*i)->load(rename);
                break;
            }
        }
    }

    // Return the named buffer.

    if (buffers.find(name) != buffers.end())
        return buffers[name]->get(len);
    else
        return 0;
}

bool app::data::save(std::string name, const void *ptr, size_t *len)
{
    // Search the list of archives for the first one that can save this buffer.

    for (archive_c i = archives.begin(); i != archives.end(); ++i)
        if ((*i)->save(name, ptr, len))
            return true;

    return false;
}

void app::data::list(std::string name, strset& dirs, strset& regs) const
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

//=============================================================================
