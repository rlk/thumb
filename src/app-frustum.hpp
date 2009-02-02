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

#ifndef APP_FRUSTUM_HPP
#define APP_FRUSTUM_HPP

#include <vector>

#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class event;
}

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------

    // USER denotes tracker coordinates. VIEW denotes world coordinates. Thus,
    // VIEW_PLANES is a cache of the view-transformed USER_PLANES, with a fifth
    // plane for use in horizon culling.

    class frustum
    {
    private:

        // Serialization node

        app::node node;

        // Current view point

        double user_pos[3];
        double view_pos[3];
        double disp_pos[3];

        double user_dist;

        // Frustum bounding planes and points

        double user_points[4][3];
        double view_points[8][3];
        double user_planes[4][4];
        double view_planes[5][4];

        double user_basis[16];
        double user_angle;

        int    view_count;
        int    pixel_w;
        int    pixel_h;

        // Projection transform

        double P[16];
        double T[16];

        // Utility functions

        void get_calibration(double&, double&, double&, double&,
                             double&, double&, double&, double&);
        void set_calibration(double,  double,  double,  double,
                             double,  double,  double,  double);
        void mat_calibration(double *);

        void calc_corner_4(double *,
                           double *,
                           double *,
                           double *, double, double);
        void calc_corner_1(double *, const double *,
                                     const double *,
                                     const double *);
        void calc_calibrated();

    public:

        frustum(app::node, int, int);
        frustum(frustum&);

        // View state mutators

        void calc_projection (double, double);
        void calc_view_points(double, double);
        void calc_user_planes(const double *);
        void calc_view_planes(const double *,
                              const double *);

        void set_horizon(double);

        // Visibility testers

        int test_bound(const double *,
                       const double *);
        int test_shell(const double *, 
                       const double *, 
                       const double *, double, double) const;
        int test_cap  (const double *, double, double, double) const;

        // Queries.

        const double *get_user_pos() const { return user_pos; }
        const double *get_view_pos() const { return view_pos; }
        const double *get_disp_pos() const { return disp_pos; }
        const double *get_P()        const { return P;        }

        double get_w()        const;
        double get_h()        const;
        int    get_pixel_w()  const { return pixel_w; }
        int    get_pixel_h()  const { return pixel_h; }
        double pixels(double) const;

        const double *get_planes() const { return view_planes[0]; }
        const double *get_points() const { return view_points[0]; }

        // Event handlers

        bool pointer_to_3D(event *, int,  int)  const;
        bool pointer_to_2D(event *, int&, int&) const;
        bool process_event(event *);

        // Perspective projection application

        void draw() const;
        void cast() const;
        void rect() const;

        void overlay() const;
    };

    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
