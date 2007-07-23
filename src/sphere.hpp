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

#include "opengl.hpp"
#include "texture.hpp"
#include "geogen.hpp"
#include "georen.hpp"
#include "geomap.hpp"
#include "patch.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class sphere
    {
        // Seed patches

        patch  *C[20];
        point  *P[12];
        GLsizei count;

        // Min and max radius

        double r0;
        double r1;

        // Current position, north vector, rotation angle

        double p[3];
        double n[3];
        double a;

        // View vector, object-to-eye transform and inverse cache.

        double v[3];
        double MVP[16];
        double M[16];
        double I[16];
        double V[24];

        // Current eye distance and depth range.

        bool   visible;
        double dist;
        double z0;
        double z1;

        // LOD bias, patch recursion depth, cache size.

        double  bias;
        GLsizei cache;
        int     frame;

        // OpenGL state

        geodat& dat;
        geotex  tex;
        geonrm  nrm;
        geopos  pos;
        geoacc  acc;
        geoext  ext;
        geovtx  vtx;
        georen& ren;

        geomap& color;
        geomap& normal;
        geomap& height;

        void transform(const double *,
                       const double *,
                       const double *);

    public:
    
        sphere(geodat&, georen&, geomap&, geomap&, geomap&,
               double, double, double=2.0, GLsizei=128);
       ~sphere();

        void turn(double=0);
        void move(double=0, double=0, double=0,
                  double=0, double=1, double=0, double=0);

        // Rendering pipeline.

        void view(const double *, const double *, const double *);
        void step();
        void prep();
        void pass();
        void draw();
        void xfrm();
        void wire();
        void getz(double&, double&);

        // State queries.

        double distance() const { return dist;       }
        double altitude() const { return dist - r0;  }
        double min_z()    const { return z0;         }
        double max_z()    const { return z1;         }
        int    maxcount() const { return int(count); }
    };

    bool sphcmp(const sphere *, const sphere *);
}

//-----------------------------------------------------------------------------

#endif
