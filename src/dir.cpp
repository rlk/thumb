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
#include <stdlib.h>
#include <errno.h>

#include <default.hpp>
#include <dir.hpp>

//-----------------------------------------------------------------------------

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <sys/param.h>

static bool get_app_res_path(std::string& path)
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
        path = std::string(sys_path) + "/" + std::string(res_path);

    return true;
}
#endif // __APPLE__

//-----------------------------------------------------------------------------
#ifndef _WIN32

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

//-----------------------------------------------------------------------------
#else // _WIN32

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

static bool dir_make(std::string& dir)
{
    return (CreateDirectory(dir.c_str(), 0) != 0);
}

#endif
//-----------------------------------------------------------------------------

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
        if (reg || dir_make(path))
            return true;
        else
            return false;
    }
    else return false;
}

//-----------------------------------------------------------------------------

// Identify a valid data directory as one containing the default data file.

static bool is_data_dir(std::string dir)
{
    std::string file(dir + "/" + DEFAULT_DATA_FILE);

    if (is_dir(dir) && is_reg(file))
        return true;
    else
        return false;
}

// Locate a read-only data hierarchy.

bool find_ro_data(std::string& path)
{
    // Check the environment variable.

    if (char *data = getenv("THUMB_RO_DATA"))
    {
        path = data;
        return true;
    }

#ifdef __APPLE__
    std::string test;

    // Check for a MacOS .app bundle hierarchy.

    if (get_app_res_path(test))
    {
        if (is_data_dir(test + "/data"))
        {
            path = test + "/data";
            return true;
        }
    }
#endif

    // Check the current working directory.

    if (is_data_dir("data"))
    {
        path = "data";
        return true;
    }

    // Check the system share directory.

    if (is_data_dir("/usr/share/thumb/data"))
    {
        path = "/usr/share/thumb/data";
        return true;
    }

    // Check the system local share directory.

    if (is_data_dir("/usr/local/share/thumb/data"))
    {
        path = "/usr/local/share/thumb/data";
        return true;
    }

    return false;
}

// Locate a writable data hierarchy.

bool find_rw_data(std::string& path)
{
    // Check for a home directory.

    if (char *home = getenv("HOME"))
    {
        path = std::string(home) + "/.thumb";
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
