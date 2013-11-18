//  Copyright (C) 2007-2011 Robert Kooima
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

#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class event;
}

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    /// 3D frustum
    ///
    /// A frustum object represents an off-axis 3D pyramid defined by five
    /// points in space, with four points giving the base of the pyramid and
    /// one point giving its apex. This most often represents a user's field of
    /// view, where the four corner positions of a display screen give the
    /// base, and the position of the user's eye give the apex.
    ///
    /// The frustum object provides functions to test whether simple geometric
    /// shapes intersect with the volume of the pyramid, and in so doing it
    /// tests whether these shapes are visible to the user. To support view
    /// determination during 3D navigation, the implementation caches its 3D
    /// corner positions in both user space and world space, updating its
    /// internal state as the user's position and viewing transformation
    /// change.
    ///
    /// Finally, the frustum provides mechanisms for display configuration. It
    /// implements the app::host::process_event protocol to allow run-time
    /// calibration, and serializes its state to the XML DOM provided at
    /// construction.

    class frustum
    {
    private:

        // Serialization node

        app::node node;

        const int pixel_w;
        const int pixel_h;

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

    public:

        frustum(app::node node, int w, int h);
        frustum(const frustum& that);

        // View state mutators

        void set_distances (double n, double f);
        void set_projection(const double *M);
        void set_viewpoint (const double *p);
        void set_transform (const double *M);
        void set_horizon   (double r);

        void set_volume(int frusc, const frustum *const *frusv,
                        double c0, double c1, const double *L, double *M,
                                                               double *I);

        // Visibility testers

        int test_bound(const double *,
                       const double *);
        /*
        int test_shell(const double *,
                       const double *,
                       const double *, double, double) const;
        int test_cap  (const double *, double, double, double) const;
        */

        // Queries.

        const double *get_user_pos()    const;
        const double *get_view_pos()    const;
        const double *get_disp_pos()    const;
        const double *get_proj_matrix() const;

        double get_w()          const;
        double get_h()          const;
        int    get_pixel_w()    const;
        int    get_pixel_h()    const;
        double pixels(double a) const;

        const double *get_planes() const { return view_planes[0]; }
        const double *get_points() const { return view_points[0]; }

        double get_n_dist() const { return n_dist; }
        double get_f_dist() const { return f_dist; }

        double get_split_coeff(double) const;
        double get_split_fract(double) const;
        double get_split_depth(double) const;

        // Event handlers

        bool pointer_to_3D(event *, int,  int)  const;
        bool pointer_to_2D(event *, int&, int&) const;
        bool process_event(event *);

        // Perspective projection application

        void load_transform() const;

        void overlay() const;

    private:

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
        void calc_basis();

    };

    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
