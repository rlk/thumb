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
    public:

        enum view_type { type_mono, type_varrier, type_anaglyph };
        enum view_mode { mode_norm, mode_test };

    private:

        // TODO: eliminate W and H

        int    w;
        int    h;
        double n;
        double f;

        // Modelview and projection matrix caches

        double default_M[16];
        double current_M[16];
        double current_P[16];

        // View configuration and cache

        double P[3];            // View position
        double X[3];            // View right vector

        double R[3];            // Screen right  vector
        double U[3];            // Screen up     vector
        double N[3];            // Screen normal vector

        double BL[3];           // Screen bottom-left  position
        double BR[3];           // Screen bottom-right position
        double TL[3];           // Screen top-left     position
        double TR[3];           // Screen top-right    position

        enum view_type type;
        enum view_mode mode;

        void find_P();

    public:

        view(int, int, double, double);

        double get_n() const { return n; }
        double get_f() const { return f; }
        int    get_w() const { return w; }
        int    get_h() const { return h; }

        void clr();
        void get_M(      double *);
        void get_P(      double *);
        void set_M(const double *);
        void set_P(const double *);
        void set_V(const double *, const double *,
                   const double *, const double *);

        void turn(const double *, const double *);
        void turn(double, double, double);
        void move(double, double, double);
        void home();

        void mult_S() const;
        void mult_P() const;
        void mult_O() const;
        void mult_M() const;
        void mult_R() const;
        void mult_T() const;
        void mult_V() const;

        void plane_frustum(double *) const;
        void point_frustum(double *) const;

        void set_type(enum view_type t) { type = t; }
        void set_mode(enum view_mode t) { mode = t; }

        enum view_type get_type() const { return type; }
        enum view_mode get_mode() const { return mode; }
        
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
