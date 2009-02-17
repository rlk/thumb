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

#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <set>

#include <cmath>

// TODO: Put this stuff where it belongs.

//-----------------------------------------------------------------------------

#ifndef CLAMP
#define CLAMP(n, a, b) std::min(std::max(n, a), b)
#endif

//-----------------------------------------------------------------------------

#ifdef _WIN32
#define usleep(t) Sleep(t)
#endif

//-----------------------------------------------------------------------------

typedef std::vector<std::string> str_vec;
typedef std::set   <std::string> str_set;

//-----------------------------------------------------------------------------

#define get_bit(b, i) (((b) >> ((i)    )) & 1)
#define get_oct(b, i) (((b) >> ((i) * 3)) & 7)

#define set_bit(b, i, n) (((b) & (~(1 << ((i)    )))) | ((n) << ((i)    )))
#define set_oct(b, i, n) (((b) & (~(7 << ((i) * 3)))) | ((n) << ((i) * 3)))

//-----------------------------------------------------------------------------

#define min4(a, b, c, d) std::min(std::min(a, b), \
                                  std::min(c, d))
#define max4(a, b, c, d) std::max(std::max(a, b), \
                                  std::max(c, d))

#define min8(a, b, c, d, e, f, g, h) std::min(min4(a, b, d, c), \
                                              min4(e, f, g, h))
#define max8(a, b, c, d, e, f, g, h) std::max(max4(a, b, d, c), \
                                              max4(e, f, g, h))

//-----------------------------------------------------------------------------

float ntohf(float);
int   ntohi(int);

void nodelay(int);

int next_power_of_2(int);

int nearest_int(double);

//-----------------------------------------------------------------------------

#endif
