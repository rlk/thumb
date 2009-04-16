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

#include "app-file.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class event;
}

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------

    // USER denotes tracker coordinates. VIEW denotes world coordinates. DISP
    // denotes display-space coordinates. Thus, VIEW_POINTS is a cache of the
    // view-transformed USER_POINTS.

    class frustum
    {
    private:

        // Serialization node

        app::node node;

        int pixel_w;
        int pixel_h;

        // Current view point

        double user_pos[3];
        double view_pos[3];
        double disp_pos[3];

        // Frustum bounding planes and points

        double user_points[4][3];
        double view_points[8][3];
        double view_planes[6][4];  // N L R B T H

        double user_basis[16];
        double user_angle;
        int    view_count;

        double n_dist;
        double f_dist;

        // Projection transform

        double P[16];

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
        frustum(const frustum&);

        // View state mutators

        void set_distances(double, double);
        void set_viewpoint(const double *);
        void set_transform(const double *);
        void set_horizon  (double);

        void calc_union(int, const frustum *const *, double,   double,
                             const double  *,        double *, double *);

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

        double get_split_coeff(double) const;
        double get_split_fract(double) const;
        double get_split_depth(double) const;

        // Event handlers

        bool pointer_to_3D(event *, int,  int)  const;
        bool pointer_to_2D(event *, int&, int&) const;
        bool process_event(event *);

        // Perspective projection application

        void draw() const;

        void overlay() const;
    };

    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
