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

#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <vector>
#include <mxml.h>

#include "matrix.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class frustum
    {
    private:

        typedef double point[3];

        point BL;
        point BR;
        point TL;
        point TR;

        std::vector<point> P;

    public:

        frustum(mxml_node_t *);

        void set_P(const double *, int);

        void get_matrix(int, double *) const;
        int  get_points(int, double *) const;
        int  get_planes(int, double *) const;

        void draw(int) const;
    };
}

//-----------------------------------------------------------------------------

#endif
