//  Copyright (C) 2005-2011 Robert Kooima
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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <set>

#include <cmath>

// TODO: Put this stuff where it belongs.

//-----------------------------------------------------------------------------

#ifdef _WIN32
#define usleep(t) Sleep(t)
#endif

//-----------------------------------------------------------------------------

typedef std::vector<std::string> str_vec;
typedef std::set   <std::string> str_set;

//-----------------------------------------------------------------------------

#endif
