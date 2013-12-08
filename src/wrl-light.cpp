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

#include <wrl-light.hpp>

//-----------------------------------------------------------------------------

wrl::light::light(std::string fill) : sphere(fill)
{
}

//-----------------------------------------------------------------------------

void wrl::light::save(app::node node)
{
    // Create a new light element.

    app::node n("geom");

    n.set_s("class", "light");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
