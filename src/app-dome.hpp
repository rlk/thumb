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

#ifndef DOME_HPP
#define DOME_HPP

#include "app-disp.hpp"
#include "ogl-program.hpp"
#include "ogl-texture.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class dome : public disp
    {
        int      index;
        double   radius;

        frustum *proj_frust;
        frustum *user_frust;

        int x;
        int y;
        int w;
        int h;

        double tk[3];
        double pk[3];

        const ogl::program *draw_P;
        const ogl::program *test_P;

        const ogl::texture *grid;

    public:

        dome(app::node, app::node, const int *);

        bool input_point(int, const double *, const double *);
        bool input_click(int, int, int, bool);
        bool input_keybd(int, int, int, bool);

        virtual bool pick(double *, double *, int, int);
        virtual void prep(view_v&, frustum_v&);
        virtual void draw(view_v&, int&, bool, bool);

        virtual ~dome();
    };
}

//-----------------------------------------------------------------------------

#endif
