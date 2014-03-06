//  Copyright (C) 2005-2014 Robert Kooima
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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <app-default.hpp>
#include <app-data-file.hpp>
#include <app-conf.hpp>
#include <etc-dir.hpp>

//-----------------------------------------------------------------------------

#ifdef _WIN32
#include <io.h>
#define open  _open
#define read  _read
#define write _write
#define close _close
#endif

//-----------------------------------------------------------------------------

app::file_buffer::file_buffer(std::string name)
{
    struct stat info;
    int fd;

    // Open the named file for reading and determine its size.

#ifndef _WIN32
    if ((fd = open(name.c_str(), O_RDONLY)) == -1)
        throw open_error(name);
#else
    if ((fd = open(name.c_str(), O_RDONLY | O_BINARY)) == -1)
        throw open_error(name);
#endif

    if (fstat(fd, &info) != 0)
        throw stat_error(name);

    // Initialize the buffer.

    len = (size_t) info.st_size;
    ptr = new unsigned char[len + 1];

    memset(ptr, 0, len + 1);

    // Read all data.

    if (read(fd, ptr, len) < (int) len)
        throw read_error(name);

    close(fd);
}

//-----------------------------------------------------------------------------

// Determine whether the named file exists within this archive.

bool app::file_archive::find(std::string name) const
{
    std::string curr = pathname(path, name);

    struct stat info;

    if (stat(curr.c_str(), &info) == 0 && (info.st_mode & S_IFMT) == S_IFREG)
        return true;
    else
        return false;
}

// Return a buffer containing the named data file.

app::buffer_p app::file_archive::load(std::string name) const
{
    return new file_buffer(pathname(path, name));
}

// Save the given buffer.

bool app::file_archive::save(std::string name,
                             const void *ptr, size_t *len) const
{
    if (!name.empty())
    {
        std::string curr = pathname(path, name);

        // Allow absolute path name save only on root file system.

        if (name[0] == '/' && !path.empty())
            return false;

        // Ensure the archive is writable and the directory exists.

        if (writable && mkpath(curr, true))
        {
            size_t count = len ? (*len) : strlen((const char *) ptr);
            int fd;

            // Open the named file for writing.

            if ((fd = open(curr.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0666)) == -1)
                throw open_error(name);

            // Write all data.

            if (write(fd, ptr, count) < (int) count)
                throw write_error(name);

            close(fd);

            return true;
        }
    }
    return false;
}

// Populate lists of all directories and regular files at the given path.

void app::file_archive::list(std::string name, str_set& dirs,
                                               str_set& regs) const
{
    dir(pathname(path, name), dirs, regs);
}

//-----------------------------------------------------------------------------

