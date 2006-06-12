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

#include "util.hpp"

//-----------------------------------------------------------------------------

float cosi(int a)
{
    static float value[360];
    static bool  state = false;

    // If this is the first call, initialize the value array.

    if (!state)
    {
        for (int i = 0; i < 360; ++i)
            value[i] = float(cos(i * 6.28318528f / 360.0f));

        state = true;
    }

    return value[a % 360];
}

float sini(int a)
{
    static float value[360];
    static bool  state = false;

    // If this is the first call, initialize the value array.

    if (!state)
    {
        for (int i = 0; i < 360; ++i)
            value[i] = float(sin(i * 6.28318528f / 360.0f));

        state = true;
    }

    return value[a % 360];
}

//-----------------------------------------------------------------------------
