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

#ifndef WRL_CONSTRAINT_HPP
#define WRL_CONSTRAINT_HPP

#include <set>
#include <cstring>

#include <etc-vector.hpp>
#include <ogl-range.hpp>

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
    public:

        constraint();
       ~constraint();

        void set_mode(int);
        void set_axis(int);
        void set_grid(int);

        void set_transform(const mat4&);

        bool point(const vec3&, const vec3&, mat4&);
        void click(const vec3&, const vec3&);

        ogl::range prep(int, const app::frustum *const *);
        void       draw(int);

    protected:

        mat4 M;
        mat4 T;

        ogl::node *rot[10];
        ogl::node *pos[10];
        ogl::pool *pool;

        int    mode;
        int    axis;
        int    grid;
        int    grid_a;
        double grid_d;

        vec3   mouse_p;
        vec3   mouse_v;
        double mouse_x;
        double mouse_y;
        double mouse_a;
        double mouse_d;

        void calc_rot(double&, double&, const vec3&, const vec3&) const;
        void calc_pos(double&, double&, const vec3&, const vec3&) const;

        void draw_rot(int) const;
        void draw_pos(int) const;

        void orient();
    };
}

//-----------------------------------------------------------------------------

#endif
