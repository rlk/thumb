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

#ifndef VIEW_HPP
#define VIEW_HPP

#include <iostream>

//-----------------------------------------------------------------------------

namespace app
{
    class view
    {
        int    w;
        int    h;
        double n;
        double f;
        double z;

        double default_M[16];
        double current_M[16];

    public:

        view(int, int, double, double, double);

        double get_n() const { return n; }
        double get_f() const { return f; }

        void clr();
        void set(const double *);
        void get(      double *);

        void turn(double, double, double);
        void move(double, double, double);
        void zoom(double);

        void mult_S() const;
        void mult_P() const;
        void mult_O() const;
        void mult_M() const;
        void mult_R() const;
        void mult_T() const;
        void mult_V() const;

        void world_frustum(double *) const;
        void plane_frustum(double *) const;
        void point_frustum(double *) const;

        void range(double, double);

        void draw() const;
        void push() const;
        void pop()  const;

        void   pick(double *, double *, int, int) const;
        double dist(double *)                     const;
    };
}

//-----------------------------------------------------------------------------

extern app::view *view;

//-----------------------------------------------------------------------------

#endif
