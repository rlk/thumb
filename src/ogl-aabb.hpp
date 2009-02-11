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

#ifndef OGL_AABB_HPP
#define OGL_AABB_HPP

#include "ogl-range.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class aabb
    {
        double a[3];
        double z[3];

        double min(const double *)                 const;
        double max(const double *)                 const;
        double max(const double *, const double *) const;

    public:

        aabb();

        void merge(double, double, double);
        void merge(const aabb&);

        ogl::range get_range(const double *, const double *) const;

        bool   test(const double *, int, 
                    const double *, int&) const;
        void   draw() const;

        double length(int i)       const { return z[i] - a[i]; }
        void   center(double c[3]) const { c[0] =  0.5 * (a[0] + z[0]);
                                           c[1] =  0.5 * (a[1] + z[1]);
                                           c[2] =  0.5 * (a[2] + z[2]); }
        void   offset(double c[3]) const { c[0] = -0.5 * (a[0] + z[0]);
                                           c[1] = -0.5 * (a[1] + z[1]);
                                           c[2] = -0.5 * (a[2] + z[2]); }
    };
}

//-----------------------------------------------------------------------------

#endif
