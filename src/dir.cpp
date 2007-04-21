//  Copyright (C) 2007 Robert Kooima
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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "dir.hpp"

#ifndef _WIN32 //--------------------------------------------------------------

#include <dirent.h>

static DIR *D = 0;

static std::string dir_list(std::string& dir)
{
    struct dirent *ent;

    if (D == 0)
        D = opendir(dir.c_str());

    if (D && (ent = readdir(D)))
        return ent->d_name;

    return "";
}

static void dir_done()
{
    if (D) closedir(D);
    D = 0;
}

#else // _WIN32 ---------------------------------------------------------------

#include <windows.h>

static HANDLE          H = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATA D;

static std::string dir_list(std::string& dir)
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

static void dir_done()
{
    FindClose(H);
    H = INVALID_HANDLE_VALUE;
}

#endif //----------------------------------------------------------------------

static bool is_dir(std::string& name)
{
    struct stat info;

    if (stat(name.c_str(), &info) == 0)
        return ((info.st_mode & S_IFMT) == S_IFDIR);
    else
        return false;
}

static bool is_reg(std::string& name)
{
    struct stat info;

    if (stat(name.c_str(), &info) == 0)
        return ((info.st_mode & S_IFMT) == S_IFREG);
    else
        return false;
}

//-----------------------------------------------------------------------------
void dir(std::string path, std::set<std::string>& dirs,
                           std::set<std::string>& regs)
{
    std::string file;
    std::string name;

    // Scan the named directory.

    while ((name = dir_list(path)).length() > 0)
    {
        // Store the names of all non-hidden files and subdirectories.

        if (name[0] != '.')
        {
            file = path + "/" + name;

            if (is_dir(file)) dirs.insert(name);
            if (is_reg(file)) regs.insert(name);
        }
    }
    dir_done();
}

bool mkpath(std::string path, bool reg)
{
    std::string::size_type s = path.rfind("/");

    // If we're at the root then we're successful.

    if (s == std::string::npos)
        return true;

    // Determine the enclosing directory path.

    std::string where = path;
    where.erase(where.rfind("/"));

    // Check whether the enclosing directory exists.

    if (is_dir(where) || mkpath(where, false))
    {
        if (reg || mkdir(path.c_str(), 0777) == 0)
            return true;
        else
            return false;
    }
    else return false;
}

//-----------------------------------------------------------------------------
