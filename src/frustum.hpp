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

        typedef double point[3];
        typedef double plane[4];

        // Serialization XML nodes

        mxml_node_t *node;

        // Frustum bounding planes

        std::vector<plane> eye_planes;
        std::vector<plane> wrl_planes;

        // The 5 points of a frustum

        point BL;
        point BR;
        point TL;
        point TR;
        point VP;

        // Calibration transform and view transform cache

        double T[16];
        double M[16];

        // Near and for clipping distances

        double n;
        double f;

        // Utility functions

        void calc_corner_4(double, double);
        void calc_corner_1(double *, const double *,
                                     const double *,
                                     const double *);
        void calc_planes();

    public:

        frustum(mxml_node_t *);
        frustum(const double *);
       ~frustum();

        // Calibration input handlers.

        void input_point(double, double);
        void input_click(int, int, bool);
        void input_keybd(int, int, bool);

        // Visibility testers.

        void test_shell(const double *, double, double, int);
        void test_bound(const double *,
                        const double *);

        // Perspective projection application.

        void draw() const;
    };
}

//-----------------------------------------------------------------------------

#endif
