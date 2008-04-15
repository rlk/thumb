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

#ifndef ATMO_HPP
#define ATMO_HPP

#include "ogl-pool.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class atmo
    {
        const ogl::program *atmo_in;
        const ogl::program *atmo_out;
        const ogl::program *land_in;
        const ogl::program *land_out;

        ogl::pool *pool;
        ogl::node *node;
        ogl::unit *unit;

        double r0;
        double r1;

    public:

        atmo(double, double);
       ~atmo();

        void draw();
    };
}

//-----------------------------------------------------------------------------

#endif
