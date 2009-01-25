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

#ifndef APP_TILE_HPP
#define APP_TILE_HPP

#include <vector>

#include "app-serial.hpp"
#include "dpy-display.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    // Forward declarations

    class event;

    // Display tile

    class tile
    {
    private:

        int            index;
        int            current;
        dpy::display_v display;

    public:

        tile(app::node);
       ~tile();

        // Rendering handlers.

        void prep(view_v&, frustum_v&);
        void draw(view_v&, int&, bool, int);

        // Event handlers.

        bool project_event(event *, int, int);
        bool process_event(event *);

        bool is_index(int i) const { return (i == index); }

        void cycle() { current = (current + 1) % display.size(); }
    };

    typedef std::vector<tile *>           tile_v;
    typedef std::vector<tile *>::iterator tile_i;
}

//-----------------------------------------------------------------------------

#endif
