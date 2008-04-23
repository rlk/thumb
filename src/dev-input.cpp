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

#include "dev-mouse.hpp"

//-----------------------------------------------------------------------------

bool dev::input::point(int i, const double *p, const double *q)
{
    return false;
}

bool dev::input::click(int i, int b, int m, bool d)
{
    return false;
}

bool dev::input::keybd(int c, int k, int m, bool d)
{
    return false;
}

bool dev::input::value(int d, int a, double v)
{
    return false;
}

bool dev::input::timer(int t)
{
    return false;
}

//-----------------------------------------------------------------------------
