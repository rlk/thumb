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

// A frustum has a life-cycle that repeats each frame. It is initially defined
// by its shape which, in the case of a generalized frustum, may be variable. As
// a cycle begins, it is given a viewing transform that determines its bounding
// volume. It then participates in visibility determination, for which bounding
// planes are computed. It is then given a content boundary, from which near and
// far distances are decided. Finally, a final projection matrix may be found
// and used for rendering.

// set_eye
// set_view
//     cache_planes
// do visibility
// set_bound
//     cache_points
// do rendering

namespace app
{
    //-------------------------------------------------------------------------
#if 1
    class frustum
    {
    public:

        frustum();

        virtual ~frustum() { }

        virtual void set_eye  (const vec3&);
        virtual void set_view (const mat4&);
        virtual void set_bound(const mat4&, const ogl::aabb&);

        // Queries

        virtual mat4  get_transform() const = 0;
        virtual void load_transform() const;

        const vec4 *get_planes() const { return plane; }
        const vec3 *get_points() const { return point; }

        // Event handlers

        virtual bool pointer_to_3D(event *, double,  double)  const;
        virtual bool pointer_to_2D(event *, double&, double&) const;
        virtual bool process_event(event *);

        // Parallel-split handlers

        double    get_split_c    (int, int) const;
        double    get_split_k    (int, int) const;
        double    get_split_z    (int, int) const;
        ogl::aabb get_split_bound(int, int) const;

    protected:

        // Frustum shape and orientation

        mat4 basis;
        vec3 corner[4];
        vec3 eye;

        // Clipping distances

        double n;
        double f;

        // World-space frustum boundary cache

        vec4 plane[6];        // N L R B T F
        vec3 point[8];        // BL BR TL TR

        void cache_basis();
        void cache_planes(const mat4&);
        void cache_points(const mat4&);
    };

    //-------------------------------------------------------------------------

    class orthogonal_frustum : public frustum
    {
    public:

        orthogonal_frustum(const ogl::aabb&, const vec3&);

        virtual mat4 get_transform() const;
    };

    //-------------------------------------------------------------------------

    class perspective_frustum : public frustum
    {
    public:

        perspective_frustum();
        perspective_frustum(const mat4&);
        perspective_frustum(const vec3&, const vec3&, double, double);

        virtual mat4 get_transform() const;
    };

    //-------------------------------------------------------------------------

    class calibrated_frustum : public perspective_frustum
    {
    public:

        calibrated_frustum(app::node);

        virtual bool process_event(event *);

    protected:

        app::node node;

        struct calibration
        {
            double P;  // Position phi
            double T;  // Position theta
            double R;  // Position rho
            double p;  // Rotation pitch
            double y;  // Rotation yaw
            double r;  // Rotation roll
            double H;  // Horizontal field of view
            double V;  // Vertical   field of view
        };

        mat4 calibration_matrix(calibration&);

        void apply_calibration();
        void  load_calibration(calibration&);
        void  save_calibration(calibration&);
    };

    //-------------------------------------------------------------------------

#else

    class frustum
    {
    public:

        frustum(app::node node);
        frustum(const frustum& that);
        frustum(const ogl::aabb&, const vec3&);

        // View state mutators

        void set_projection(const mat4& M);
        void set_eye (const vec3& p);
        void set_transform (const mat4& M);
        void set_distances (const ogl::aabb& bound);

        // Queries.

        const vec3 get_user_pos()    const;
        const vec3 get_view_pos()    const;
        const vec3 get_disp_pos()    const;
        const mat4 get_transform() const;

        double get_w()          const;
        double get_h()          const;

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

        bool pointer_to_3D(event *, double,  double)  const;
        bool pointer_to_2D(event *, double&, double&) const;
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
#endif
    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

//-----------------------------------------------------------------------------

#endif
