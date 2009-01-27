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

#include <vector>

#include "dpy-display.hpp"
#include "app-serial.hpp"

//-----------------------------------------------------------------------------
// Forward declarations

namespace ogl
{
    class program;
}

//-----------------------------------------------------------------------------

namespace dpy
{
    // A "normal" display is an ordinary 2D screen.  It is monoscopic, with
    // only a single frustum, rendering directly to the onscreen framebuffer
    // using only a single view.  It's only configuration option is which of
    // multiple views it uses.

    class normal : public display
    {
        int           chani;
        app::frustum *frust;

        const ogl::program *P;

        virtual bool process_start(app::event *);
        virtual bool process_close(app::event *);

    public:

        normal(app::node);

        virtual void get_frusta(std::vector<app::frustum *>&);

        // Rendering handlers.

        virtual void prep(dpy::channel **, int);
        virtual int  draw(dpy::channel **, int, int);
        virtual void test(dpy::channel **, int, int);

        // Event handers.

        virtual bool project_event(app::event *, int, int);
        virtual bool process_event(app::event *);

        virtual ~normal();
    };
}

//-----------------------------------------------------------------------------

#endif
