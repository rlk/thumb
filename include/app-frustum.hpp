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

namespace app
{
    //-------------------------------------------------------------------------

    class frustum
    {
    public:

        frustum();

        virtual ~frustum() { }

        virtual void set_eye  (const vec3&);
        virtual void set_view (const mat4&);
        virtual void set_bound(const mat4&, const ogl::aabb&) = 0;

        // Queries

        virtual void load_transform() const;
        virtual mat4  get_transform() const = 0;

        const vec4 *get_world_planes() const { return plane;  }
        const vec3 *get_world_points() const { return point;  }
        const vec3 *get_corners()      const { return corner; }
        const vec3  get_eye()          const { return eye;    }

        double get_width()  const { return length(corner[1] - corner[0]); }
        double get_height() const { return length(corner[2] - corner[0]); }

        // Event handlers

        virtual bool pointer_to_3D(event *, double,  double)  const;
        virtual bool pointer_to_2D(event *, double&, double&) const;
        virtual bool process_event(event *) { return false; }

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

        virtual void set_bound(const mat4&, const ogl::aabb&);

        virtual mat4 get_transform() const;
    };

    //-------------------------------------------------------------------------

    class perspective_frustum : public frustum
    {
    public:

        perspective_frustum(const vec3&, const vec3&, double, double);
        perspective_frustum(const mat4&);
        perspective_frustum();

        virtual void set_bound(const mat4&, const ogl::aabb&);

        virtual mat4 get_transform() const;
    };

    //-------------------------------------------------------------------------

    class calibrated_frustum : public perspective_frustum
    {
    public:

        calibrated_frustum(app::node);
        calibrated_frustum();

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

    typedef std::vector<frustum *>                 frustum_v;
    typedef std::vector<frustum *>::const_iterator frustum_i;

    //-------------------------------------------------------------------------
}

#endif
