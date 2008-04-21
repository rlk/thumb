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

#ifndef SPATCH_HPP
#define SPATCH_HPP

#include "app-frustum.hpp"
#include "uni-geogen.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    struct spatch
    {
        double n[3][3];
        double t[3][2];
        int    i[3][2];
        int    d;
        double k;
        double N[3];
        double a;

        void init(const double *, const double *, int, 
                  const double *, const double *, int,
                  const double *, const double *, int,
                  const double *, double, int);

        void link(int, int, int);

        bool test(const double *,
                  const double *,
                  const double *, double, double, app::frustum_v&) const;

        void seed(int,
                  geonrm&,
                  geopos&,
                  geotex&,
                  double, const double *, const double *) const;

        bool able(const spatch *) const;
        bool less(const spatch *) const;
        bool more(const spatch *) const;
        void draw(const spatch *, int, GLsizei, GLsizei, GLsizei) const;
        void wire(double, double) const;

        bool subd(spatch *, int, int&, int, double, double, app::frustum_v&, const double *);
    };
}

//-----------------------------------------------------------------------------

#endif
