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

#ifndef DPY_DOME_HPP
#define DPY_DOME_HPP

#include "dpy-display.hpp"
#include "ogl-program.hpp"

//-----------------------------------------------------------------------------

namespace dpy
{
    class dome : public display
    {
        int    index;
        double radius;

        app::frustum *proj_frust;
        app::frustum *user_frust;

        int x;
        int y;
        int w;
        int h;

        double tk[3];
        double pk[3];
        double zk[3];

        const ogl::program *draw_P;
        const ogl::program *test_P;

    public:

        dome(app::node, app::node, const int *);

        // Calibration handlers.

        virtual bool input_point(int, const double *, const double *);
        virtual bool input_click(int, int, int, bool);
        virtual bool input_keybd(int, int, int, bool);

        // Rendering handlers.

        virtual bool pick(double *, double *, int, int);
        virtual void prep(app::view_v&, app::frustum_v&);
        virtual void draw(app::view_v&, int&, bool, bool);

        virtual ~dome();
    };
}

//-----------------------------------------------------------------------------

#endif
