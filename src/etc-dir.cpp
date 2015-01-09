//  Copyright (C) 2007-2011 Robert Kooima
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
#include <stdlib.h>
#include <errno.h>

#include <etc-dir.hpp>

//-----------------------------------------------------------------------------
#ifndef _WIN32

// Unix-platform API for enumerating and creating directories.

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

static bool dir_make(std::string& dir)
{
    return (mkdir(dir.c_str(), 0777) == 0);
}

#else // _WIN32 ---------------------------------------------------------------

// Windows-platform API for enumerating and creating directories.

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

static HANDLE           H = INVALID_HANDLE_VALUE;
static WIN32_FIND_DATAA D;

static std::string dir_list(std::string& dir)
{
    if (H == INVALID_HANDLE_VALUE)
    {
        std::string pattern = dir + "/*";

        if ((H = FindFirstFileA(pattern.c_str(), &D)) != INVALID_HANDLE_VALUE)
            return D.cFileName;
    }
    else
    {
        if (FindNextFileA(H, &D))
            return D.cFileName;
    }
    return "";
}

static void dir_done()
{
    FindClose(H);
    H = INVALID_HANDLE_VALUE;
}

static bool dir_make(std::string& dir)
{
    return (CreateDirectoryA(dir.c_str(), 0) != 0);
}

#endif
//-----------------------------------------------------------------------------

// Determine whether the named file is a directory.

bool is_dir(const std::string& name)
{
    struct stat info;

    if (stat(name.c_str(), &info) == 0)
        return ((info.st_mode & S_IFMT) == S_IFDIR);
    else
        return false;
}

// Determine whether the named file is regular.

bool is_reg(const std::string& name)
{
    struct stat info;

    if (stat(name.c_str(), &info) == 0)
        return ((info.st_mode & S_IFMT) == S_IFREG);
    else
        return false;
}

//-----------------------------------------------------------------------------

// Replace any path separators with OS-approriate path separators.

std::string fixpath(std::string path)
{
    std::string temp(path);

    for (size_t i = 0; i < temp.size(); i++)
       if (temp[i] == '/')
           temp[i] = PATH_SEPARATOR;

       return temp;
}

// Compose a path name, being careful not to add unnecessary separators.

std::string pathname(std::string path, std::string name)
{
    if (path.empty())
        return name;
    else
    {
        if (*(path.rbegin()) == PATH_SEPARATOR)
            return path + name;
        else
            return path + PATH_SEPARATOR + name;
    }
}

// Populate lists of all directories and regular files at the given path.

void dir(std::string path, std::set<std::string>& dirs,
                           std::set<std::string>& regs)
{
    std::string file;
    std::string name;

    while ((name = dir_list(path)).length() > 0)
    {
        if (name[0] != '.')
        {
            file = pathname(path, name);

            if (is_dir(file)) dirs.insert(name);
            if (is_reg(file)) regs.insert(name);
        }
    }
    dir_done();
}

// Ensure the given path exists, creating directories as necessary. If reg then
// assume the last element in a regular file and don't create it.

bool mkpath(std::string path, bool reg)
{
    std::string::size_type s = path.rfind(PATH_SEPARATOR);

    // If we're at the root then we're successful.

    if (s == std::string::npos)
        return true;

    // Determine the enclosing directory path.

    std::string where = path;
    where.erase(where.rfind(PATH_SEPARATOR));

    // Check whether the enclosing directory exists.

    if (is_dir(where) || mkpath(where, false))
    {
        if (reg || dir_make(path))
            return true;
        else
            return false;
    }
    else return false;
}

//-----------------------------------------------------------------------------
#if 0
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <sys/param.h>

bool get_app_res_path(std::string& path)
{
    CFURLRef    url;
    CFStringRef str;

    char sys_path[MAXPATHLEN];
    char res_path[MAXPATHLEN];

    // Get the path to application bundle.

    url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

    CFStringGetCString(str, sys_path, MAXPATHLEN, CFStringGetSystemEncoding());
    CFRelease(str);
    CFRelease(url);

    // Get relative path to resources directory.

    url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);

    CFStringGetCString(str, res_path, MAXPATHLEN, CFStringGetSystemEncoding());
    CFRelease(str);
    CFRelease(url);

    // Don't concatenate them if they are the same.

    if (strcmp(sys_path, res_path) == 0)
        path = std::string(sys_path);
    else
        path = std::string(sys_path) + PATH_SEPARATOR + std::string(res_path);

    return true;
}

#endif // __APPLE__
#endif
//-----------------------------------------------------------------------------
