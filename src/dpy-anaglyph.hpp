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

#ifndef DPY_ANAGLYPH_HPP
#define DPY_ANAGLYPH_HPP

#include <vector>

#include "dpy-display.hpp"
#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class program;
}

//-----------------------------------------------------------------------------

namespace dpy
{
    class anaglyph : public display
    {
        app::frustum *frustL;
        app::frustum *frustR;

        const ogl::program *P;

        virtual bool process_start(app::event *);
        virtual bool process_close(app::event *);

    public:

        anaglyph(app::node);

        virtual void get_frustums(std::vector<app::frustum *>&);

        // Rendering handlers.

        virtual void prep(int, dpy::channel **);
        virtual int  draw(int, dpy::channel **,
                          int, app::frustum  *);
        virtual int  test(int, dpy::channel **, int);

        // Event handers.

        virtual bool pointer_to_3D(app::event *, int, int);
        virtual bool process_event(app::event *);

        virtual app::frustum *get_overlay() const { return frustL; }

        virtual ~anaglyph();
    };
}

//-----------------------------------------------------------------------------

#endif
