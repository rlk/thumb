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
#include <dirent.h>

#include "directory.hpp"

//-----------------------------------------------------------------------------

directory::directory(std::string start)
{
    std::string name;

    DIR *D;

    // Test for the given directory's existence.

    if ((D = opendir(start.c_str())))
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

        closedir(D);
    }
    else
    {
        // If the given directory doesn't exist, set a reasonable default.

        path.push_back("/");
    }
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

    DIR *D;

    if ((D = opendir(path.c_str())))
    {
        struct dirent *ent;
        struct stat    buf;

        // Scan the current working directory.

        while ((ent = readdir(D)))
        {
            std::string name(ent->d_name);
            std::string file = path + name;

            if (name == ".." || name[0] != '.')
            {
                // Store the name of all matching files and subdirectories.

                if (stat(file.c_str(), &buf) == 0)
                {
                    if (S_ISDIR(buf.st_mode))
                        dirs.push_back(name);

                    if (S_ISREG(buf.st_mode))
                    {
                        size_t dot = file.rfind(".");

                        if (std::string::npos      != dot &&
                            std::string(file, dot) == ext)
                                regs.push_back(name);
                    }
                }
            }
        }
        closedir(D);
    }
}

//-----------------------------------------------------------------------------
