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

#include <sys/types.h>
#include <sys/stat.h>

#include "directory.hpp"

//-----------------------------------------------------------------------------
// Unix directory access abstraction.

#ifndef _WIN32
#include <dirent.h>

static DIR *D = 0;

std::string dir_scan(std::string& dir)
{
    struct dirent *ent;

    if (D == 0)
        D = opendir(dir.c_str());

    if (D && (ent = readdir(D)))
        return ent->d_name;

    return "";
}

void dir_close()
{
    if (D) closedir(D);
    D = 0;
}

//-----------------------------------------------------------------------------
// Win32 directory access abstraction.

#else
#include <windows.h>

static HANDLE          H = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATA D;

std::string dir_scan(std::string& dir)
{
    if (H == INVALID_HANDLE_VALUE)
    {
        std::string pattern = dir + "*";

        if ((H = FindFirstFile(pattern.c_str(), &D)) != INVALID_HANDLE_VALUE)
            return D.cFileName;
    }
    else
    {
        if (FindNextFile(H, &D))
            return D.cFileName;
    }
    return "";
}

void dir_close()
{
    FindClose(H);
    H = INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
#endif

directory::directory(std::string start)
{
    std::string name;

    // Test for the given directory's existence.

    if (dir_scan(start).length() > 0)
    {
        // Parse the given path name into a stack of directory names.

        for (std::string::size_type i = 0; i < start.size(); ++i)

            if (start[i] != '/')
                name.push_back(start[i]);
            else
            {
                path.push_back(name);
                name.erase();
            }

        // Include any trailing directory name not ending in '/'.

        if (!name.empty()) path.push_back(name);
    }
    else
    {
        // If the given directory doesn't exist, set a reasonable default.

        path.push_back("/");
    }
    dir_close();
}

std::string directory::cwd()
{
    std::string name;

    // Compose a path string from the current directory stack.

    for (strvec::iterator i = path.begin(); i != path.end(); ++i)
        name += (*i) + "/";

    return name;
}

void directory::set(std::string name)
{
    if (name == "..")
    {
        if (path.size() > 1)
            path.pop_back();
    }
    else
        path.push_back(name);
}

void directory::get(strvec& dirs, strvec& regs, std::string& ext)
{
    std::string path = cwd();
    std::string file;
    std::string name;

    // Scan the current working directory.

    while ((name = dir_scan(path)).length() > 0)
    {
        struct stat buf;

        file = path + name;
        
        // Store the name of all matching files and subdirectories.

        if (stat(file.c_str(), &buf) == 0)
        {
            if (((buf.st_mode) & S_IFMT) == S_IFDIR)
                dirs.push_back(name);

            if (((buf.st_mode) & S_IFMT) == S_IFREG)
            {
                size_t dot = name.rfind(".");

                if (std::string::npos      != dot &&
                    std::string(name, dot) == ext)
                        regs.push_back(name);
            }
        }
    }
    dir_close();
}

//-----------------------------------------------------------------------------
