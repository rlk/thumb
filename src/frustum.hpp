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

//-----------------------------------------------------------------------------

namespace app
{
    class frustum
    {
    private:

        // Serialization XML node

        mxml_node_t *node;

        // Frustum bounding planes

        double eye_planes[4][4];
        double wrl_planes[4][4];

        // Configured frustum corners and calibrated corner cache

        double c[4][3];
        double C[4][3];

        // Near and far corner cache.

        double N[4][3];
        double F[4][3];

        // Calibration transform

        double T[16];

        // Projection transform

        double P[16];

        // Utility functions

        void calc_corner_4(double, double);
        void calc_corner_1(double *, const double *,
                                     const double *,
                                     const double *);

        void calc_calibrated();
        void calc_projection();
        void calc_eye_planes(const double *);
        void calc_wrl_planes(const double *);

    public:

        frustum(mxml_node_t *);
        frustum(const double *);
       ~frustum();

        // View state mutators

        void set_view(const double *,
                      const double *);
        void set_dist(const double *,
                      const double *, double, double);

        // Calibration input handlers

        void input_point(double, double);
        void input_click(int, int, bool);
        void input_keybd(int, int, bool);

        // Visibility testers

        void test_shell(const double *, double, double, int);
        void test_bound(const double *,
                        const double *);

        // Perspective projection application

        void draw() const;
    };
}

//-----------------------------------------------------------------------------

#endif
