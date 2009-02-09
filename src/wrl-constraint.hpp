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

#ifndef WRL_CONSTRAINT_HPP
#define WRL_CONSTRAINT_HPP

#include <set>
#include <cstring>

#include "ogl-range.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class frustum;
}

namespace ogl
{
    class unit;
    class node;
    class pool;
}

//-----------------------------------------------------------------------------

namespace wrl
{
    class constraint
    {
    protected:

        double M[16];
        double T[16];

        ogl::unit *rot[10];
        ogl::unit *pos[10];

        ogl::node *node;
        ogl::pool *pool;

        int    mode;
        int    axis;
        int    grid;
        int    grid_a;
        double grid_d;

        double mouse_x;
        double mouse_y;
        double mouse_a;
        double mouse_d;

        void calc_rot(double&, double&, const double *, const double *) const;
        void calc_pos(double&, double&, const double *, const double *) const;

        void draw_rot(int) const;
        void draw_pos(int) const;

        void orient();

    public:

        constraint();
       ~constraint();

        void set_mode(int);
        void set_axis(int);
        void set_grid(int);

        void set_transform(const double *);

        bool point(double *, const double *, const double *);
        void click(          const double *, const double *);

        ogl::range prep(int, app::frustum **);
        void       draw(int, app::frustum  *);
    };
}

//-----------------------------------------------------------------------------

#endif
