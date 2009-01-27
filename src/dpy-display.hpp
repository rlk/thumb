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

#ifndef DPY_DISPLAY_HPP
#define DPY_DISPLAY_HPP

#include <vector>

#include "app-serial.hpp"

//-----------------------------------------------------------------------------
// Forward declarations

namespace app
{
    class event;
    class frustum;
}

namespace dpy
{
    class channel;
}

//-----------------------------------------------------------------------------

namespace dpy
{
    // Interface for display handling (Mono/Varrier/Dome/Anaglyph/etc)

    class display
    {
    protected:

        int index;
        int viewport[4];

        void fill(double, double, int, int) const;

    public:

        display(app::node);

        virtual void get_frusta(std::vector<app::frustum *>&) { }

        // Rendering handlers.

        virtual void prep(dpy::channel **, int)      = 0;
        virtual int  draw(dpy::channel **, int, int) = 0;
        virtual void test(dpy::channel **, int, int) = 0;

        // Event handlers.

        virtual bool project_event(app::event *, int, int) { return false; }
        virtual bool process_event(app::event *)           { return false; }

        virtual ~display() { }
    };

    typedef std::vector<display *>           display_v;
    typedef std::vector<display *>::iterator display_i;
}

//-----------------------------------------------------------------------------

#endif
