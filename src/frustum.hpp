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

#include "serial.hpp"

//-----------------------------------------------------------------------------

// TODO: frustum should be able to evaluate LOD?
//       or at least encapsulate view position (which it sorfof does)

// TODO: technically not necessary to store user_points?

namespace app
{
    class frustum
    {
    private:

        // Serialization node

        app::node node;

        // Current view point

        double user_pos[3];
        double view_pos[3];

        double user_dist;

        // Frustum bounding planes and points

        double user_points[4][3];
        double view_points[8][3];
        double user_planes[4][4];
        double view_planes[4][4];

        // Projection transform

        double P[16];

        // Utility functions

        void get_calibration(      double *);
        void set_calibration(const double *);

        void calc_corner_4(double *,
                           double *,
                           double *,
                           double *, double, double);
        void calc_corner_1(double *, const double *,
                                     const double *,
                                     const double *);
        void calc_calibrated();

    public:

        frustum(app::node);
        frustum(frustum&);

        // View state mutators
/*
        void set_view(const double *,
                      const double *);
        void set_dist(const double *,
                      const double *, double, double);
*/
        void calc_projection (double, double);
        void calc_view_points(double, double);
        void calc_user_planes(const double *);
        void calc_view_planes(const double *,
                              const double *);

        // Calibration input handlers

        bool input_point(int, const double *, const double *);
        bool input_click(int, int, int, bool);
        bool input_keybd(int, int, int, bool);

        // Visibility testers

        int test_shell(const double *, 
                       const double *, 
                       const double *, double, double) const;
        int test_bound(const double *,
                       const double *);

        // Perspective projection application

        void draw() const;
    };

    typedef std::vector<frustum *>           frustum_v;
    typedef std::vector<frustum *>::iterator frustum_i;
}

//-----------------------------------------------------------------------------

#endif
