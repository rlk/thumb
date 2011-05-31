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

#ifndef APP_PROG_HPP
#define APP_PROG_HPP

#include <string>

#include <ogl-range.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class event;
    class frustum;
}

namespace dev
{
    class input;
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

        dev::input *input;

    public:

        prog(const std::string&);

        virtual ~prog();

        // Rendering handlers

        virtual ogl::range prep(int, const app::frustum * const *) = 0;
        virtual void       lite(int, const app::frustum * const *) = 0;
        virtual void       draw(int, const app::frustum *)         = 0;

        // Event handler

        virtual bool process_event(event *);
        virtual void run();

        // Screenshot procedure

        void screenshot(std::string, int, int) const;
    };
}

//-----------------------------------------------------------------------------

#endif
