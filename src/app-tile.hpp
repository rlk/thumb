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

//-----------------------------------------------------------------------------
// Forward declarations

namespace app
{
    class event;
}

namespace dpy
{
    class display;
}

//-----------------------------------------------------------------------------

namespace app
{
    // Display tile

    class tile
    {
    private:

        int           index;
        dpy::display *display;

    public:

        tile(app::node);
       ~tile();


        // Rendering handlers.

        void prep(view *, frustum_v&);
        void draw(view *, int);
        void test(view *, int);

        // Event handlers.

        bool project_event(event *, int, int);
        bool process_event(event *);

        bool is_index(int i) const { return (i == index); }
    };

    typedef std::vector<tile *>           tile_v;
    typedef std::vector<tile *>::iterator tile_i;
}

//-----------------------------------------------------------------------------

#endif
