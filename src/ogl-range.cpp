//  Copyright (C) 2009 Robert Kooima
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

#include <algorithm>
#include <limits>

#include "ogl-range.hpp"

//-----------------------------------------------------------------------------

ogl::range::range(double n, double f) : n(n), f(f)
{
}

ogl::range::range()
{
    n =  std::numeric_limits<double>::max();
    f = -std::numeric_limits<double>::max();
}

void ogl::range::merge(double z)
{
    n = std::min(n, z);
    f = std::max(f, z);
}

void ogl::range::merge(const range& that)
{
    n = std::min(n, that.n);
    f = std::max(f, that.f);
}

//-----------------------------------------------------------------------------
