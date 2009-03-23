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

#ifndef APP_PROG_HPP
#define APP_PROG_HPP

#include <string>

#include "ogl-range.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class event;
    class frustum;
}

//-----------------------------------------------------------------------------

namespace app
{
    // Application interface.

    class prog
    {
        int key_snap;
        int key_exit;
        int key_init;

        unsigned int options;

    public:

        void screenshot(std::string, int, int) const;

        prog();

        virtual ogl::range prep(int, app::frustum **) { return ogl::range(); }
        virtual void       draw(int, app::frustum  *) {                      }

        virtual bool process_event(event *);

        // Debug toggles.

        int  get_options() const;
        void set_options(int);

        bool get_option(int) const;
        void set_option(int);
        void clr_option(int);
        void tgl_option(int);

        virtual void next() { }
        virtual void prev() { }

        virtual ~prog() { }
    };
}

//-----------------------------------------------------------------------------

extern app::prog *prog;

//-----------------------------------------------------------------------------

#endif
