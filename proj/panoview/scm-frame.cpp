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

scm_frame::scm_frame(scm_cache *cache, app::node node) : mono(true)
{
    for (app::node n = node.find("scm"); n; n = node.next(n, "scm"))
    {
        image I;

        I.file = cache->add_file(n.get_s("file"),
                                 n.get_f("r0", 1.0),
                                 n.get_f("r1", 1.0));

        I.shader  = (n.get_s("shader") == "vert") ? 0 : 1;
        I.channel = (n.get_i("channel"));

        if (I.channel) mono = false;

        images.push_back(I);
    }
}

//------------------------------------------------------------------------------
