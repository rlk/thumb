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

#ifndef TILE_HPP
#define TILE_HPP

#include <vector>

#include "serial.hpp"
#include "disp.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class tile
    {
    private:

        int    index;
        int    window[4];

        disp_v display;
        int    current;

    public:

        tile(app::node);
       ~tile();

        bool input_point(int, const double *, const double *);
        bool input_click(int, int, int, bool);
        bool input_keybd(int, int, int, bool);

        void prep(view_v&, frustum_v&);
        void draw(view_v&, bool, int);

        bool pick(double *, double *, int, int);

        void set_type(int i) { current = i; }
        bool is_index(int i) const { return (i == index); }
    };

    typedef std::vector<tile *>           tile_v;
    typedef std::vector<tile *>::iterator tile_i;
}

//-----------------------------------------------------------------------------

#endif
