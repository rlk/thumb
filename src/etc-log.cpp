//  Copyright (C) 2011-2014 Robert Kooima
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

#include <cstdarg>
#include <cstdio>
#include <string>

#include <etc-log.hpp>

//------------------------------------------------------------------------------

#ifdef WIN32
#include <Windows.h>

void etc::log(const char *fmt, ...)
{
    char str[1024];

    va_list  ap;
    va_start(ap, fmt);
    vsnprintf(str, 1024, fmt, ap);
    va_end  (ap);

    OutputDebugStringA("(Thumb) ");
    OutputDebugStringA(str);
    OutputDebugStringA("\n");
}

#else

void etc::log(const char *fmt, ...)
{
    flockfile(stderr);
    {
        va_list  ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
         fprintf(stderr, "\n");
        va_end  (ap);
    }
    funlockfile(stderr);
}

#endif

void etc::log(const std::string& str)
{
    etc::log(str.c_str());
}

//------------------------------------------------------------------------------
