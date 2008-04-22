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

#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "default.hpp"
#include "ogl-opengl.hpp"
#include "ogl-texture.hpp"
#include "app-frustum.hpp"
#include "uni-geogen.hpp"
#include "uni-georen.hpp"
#include "uni-geocsh.hpp"
#include "uni-geomap.hpp"
#include "uni-spatch.hpp"
#include "ogl-pool.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class sphere
    {
        spatch *S;
        GLsizei count;

        // Min and max radius

        double r0;
        double r1;
        double a0;
        double a1;

        // Current position, north vector, rotation angle

        double p[3];
        double n[3];
        double angle;
        double tilt;

        // View vector, object-to-eye transform and inverse cache.

        double vp[3];
        double O[16];
        double M[16];
        double I[16];

        app::frustum_v frusta;

        // Current eye distance and depth range.

        bool   visible;
        double dist;
        double d0;
        double d1;

        // Cache size.

        GLsizei lines;

        // OpenGL state

        geodat& dat;
        geotex  tex;
        geonrm  nrm;
        geopos  pos;
        geoacc  acc;
#ifdef CONF_CALCEXT
        geoext  ext;
#endif
        geovtx  vtx;
        georen& ren;

        geocsh& cache_s;
        geocsh& cache_h;
        geomap& color;
        geomap& normal;
        geomap& height;

        const ogl::program *draw_lit;
        const ogl::program *atmo_in;
        const ogl::program *atmo_out;
        const ogl::program *land_in;
        const ogl::program *land_out;

        bool draw_atmo;

        ogl::pool *atmo_pool;
        ogl::node *atmo_node;
        ogl::unit *atmo_unit;

        void atmo_prep(const ogl::program *) const;
        void transform(app::frustum_v&);

        bool test(const double *,
                  const double *,
                  const double *) const;

    public:
    
        sphere(geodat&, georen&, geocsh&, geocsh&, geomap&, geomap&, geomap&,
               double, double, GLsizei=DEFAULT_PATCH_CACHE);
       ~sphere();

        void turn(double=0, double=0);
        void move(double=0, double=0, double=0,
                  double=0, double=1, double=0, double=0);
        void norm();

        // Rendering pipeline.

        void view(app::frustum_v&);
        void step();
        void prep();
        void pass();
        void wire();
        void draw(int);

        // State queries.

        double distance() const { return dist;       }
        double altitude() const { return dist - r0;  }
        int    maxcount() const { return int(count); }

        const double *get_p() const { return p; }

        double get_a()   const { return angle; }
        void   set_a(double k) { angle = k; norm(); }
        double get_t()   const { return tilt;  }
        void   set_t(double k) { tilt  = k; norm(); }
    };

    bool sphcmp(const sphere *, const sphere *);
}

//-----------------------------------------------------------------------------

#endif
