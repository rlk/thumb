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

#include <stdexcept>
#include <cstring>

#include <sys-util.hpp>

//-----------------------------------------------------------------------------

double scale_to_meters(const std::string& unit)
{
    // Return a scaling factor to convert the named unit to meters.

    if (unit == "mi") return 5280.0 * 12.0 * 2.54 / 100.0;
    if (unit == "yd") return    3.0 * 12.0 * 2.54 / 100.0;
    if (unit == "ft") return          12.0 * 2.54 / 100.0;
    if (unit == "in") return                 2.54 / 100.0;

    if (unit == "km") return 1000.000;
    if (unit == "cm") return    0.010;
    if (unit == "mm") return    0.001;

    return 1.0;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
