//  Copyright (C) 2008 Robert Kooima
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

#include "dpy-display.hpp"

//-----------------------------------------------------------------------------

dpy::display::display(app::node node)
{
    app::node curr;

    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = DEFAULT_PIXEL_WIDTH;
    viewport[3] = DEFAULT_PIXEL_HEIGHT;

    // Check for view and tile indices.

    index = get_attr_d(node, "index", 0);

    // Extract the window viewport rectangle.

    if ((curr = find(node, "viewport")))
    {
        viewport[0] = get_attr_d(curr, "x");
        viewport[1] = get_attr_d(curr, "y");
        viewport[2] = get_attr_d(curr, "w", DEFAULT_PIXEL_WIDTH);
        viewport[3] = get_attr_d(curr, "h", DEFAULT_PIXEL_HEIGHT);
    }
}

app::display::~display()
{
}

//-----------------------------------------------------------------------------

