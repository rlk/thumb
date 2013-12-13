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

#include <etc-vector.hpp>
#include <ogl-aabb.hpp>
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
    public:

        frustum(app::node node, int w, int h);
        frustum(const frustum& that);
        frustum(const ogl::aabb&, const vec3&);
        // frustum(const ogl::aabb&, const vec3&, const vec3&, float);

        // View state mutators

        void set_projection(const mat4& M);
        void set_viewpoint (const vec3& p);
        void set_transform (const mat4& M);
        void set_distances (const ogl::aabb& bound);

        void set_volume(int frusc, const frustum *const *frusv,
                        double c0, double c1, const vec3& p, mat4& M,
                                                             mat4& I);

        // Queries.

        const vec3 get_user_pos()    const;
        const vec3 get_view_pos()    const;
        const vec3 get_disp_pos()    const;
        const mat4 get_perspective() const;

        double get_w()          const;
        double get_h()          const;
        int    get_pixel_w()    const;
        int    get_pixel_h()    const;

        const vec4 *get_planes() const { return view_planes; }
        const vec3 *get_points() const { return view_points; }

        double get_n_dist() const { return n_dist; }
        double get_f_dist() const { return f_dist; }

        // Parallel-split handlers

        double    get_split_c    (int, int) const;
        double    get_split_k    (int, int) const;
        double    get_split_z    (int, int) const;
        ogl::aabb get_split_bound(int, int) const;

        // Event handlers

        bool pointer_to_3D(event *, int,  int)  const;
        bool pointer_to_2D(event *, int&, int&) const;
        bool process_event(event *);

        // Perspective projection application

        void load_transform() const;
        void apply_overlay()  const;

    private:

        // Utility functions

        void get_calibration(double&, double&, double&, double&,
                             double&, double&, double&, double&);
        void set_calibration(double,  double,  double,  double,
                             double,  double,  double,  double);
        mat4 mat_calibration();

        void calc_corner_4(vec3&,
                           vec3&,
                           vec3&,
                           vec3&, double, double);
        vec3 calc_corner_1(const vec3&,
                           const vec3&,
                           const vec3&);
        void calc_calibrated();
        void calc_basis();

        // Serialization node

        app::node node;

        const int pixel_w;
        const int pixel_h;

        // Current view point

        vec3 user_pos;
        vec3 view_pos;

        // Frustum bounding planes and points

        vec3 user_points[4];
        vec3 view_points[8];
        vec4 view_planes[6];  // N L R B T

        // User-space basis for the display coordinate system

        mat3 user_basis;

        // Projection transform

        mat4 P;

        double n_dist;
        double f_dist;
    };

    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
