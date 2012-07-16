//  Copyright (C) 2005-2012 Robert Kooima
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

#include "scm-frame.hpp"

//------------------------------------------------------------------------------

scm_frame::scm_frame() : mono(true)
{
}

//------------------------------------------------------------------------------

// Return true if any one of the images has page i in cache.

bool scm_frame::page_status(long long i) const
{
    return false;
}

double scm_frame::page_r0(long long i) const
{
    return 0.0;
}

double scm_frame::page_r1(long long i) const
{
    return 1.0;
}

//------------------------------------------------------------------------------
