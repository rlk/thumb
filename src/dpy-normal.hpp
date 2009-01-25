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

#ifndef DPY_NORMAL_HPP
#define DPY_NORMAL_HPP

#include "dpy-display.hpp"
#include "ogl-program.hpp"
#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace dpy
{
    // A "normal" display is an ordinary 2D screen.  It is monoscopic, with
    // only a single frustum, rendering directly to the onscreen framebuffer
    // using only a single view.  It's only configuration option is which of
    // multiple views it uses.

    class normal : public display
    {
        int           index;
        app::frustum *frust;

        int x;
        int y;
        int w;
        int h;

        const ogl::program *P;

    public:

        normal(app::node, app::node, const int *);

        // Rendering handlers.

        virtual void prep(app::view_v&, app::frustum_v&);
        virtual void draw(app::view_v&, app::frustum *, int);

        // Event handers.

        virtual int project_event(event *, int, int);
        virtual int process_event(event *);

        virtual ~normal();
    };
}

//-----------------------------------------------------------------------------

#endif
