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

#ifndef PATCH_HPP
#define PATCH_HPP

#include "opengl.hpp"
#include "frustum.hpp"
#include "geogen.hpp"

namespace uni
{
    //-------------------------------------------------------------------------

    class context;

    //-------------------------------------------------------------------------

    class point
    {
        double n[3];  // Normal vector
        double V[3];  // Eye-space vertex cache
        double N[3];  // Eye-space normal cache

        int  count;    // Refrence count
        int  frame;    // Cache frame number

    public:

        point(const double *);
        point(const point  *,
              const point  *);

        point *  ref();
        point *unref();

        void transform(const double *, const double *, double, int);
        void project  (      double *, const double *, double);

        void seed(geonrm&, geopos&, GLsizei);

        const double *get() const { return n; }

        void draw(double) const;
    };

    //-------------------------------------------------------------------------

    class patch
    {
        // Child patches and points.

        patch *C[4];
        point *P[3];

        double t[6];
        double n[3];

        double  a;
        double  area;
        double  rr;
        double  r0;
        double  r1;
        double  r2;
        GLsizei cache;

        int  visible(app::frustum_v&, const double *);
        double value(const double *);
        void   bound(GLfloat, GLfloat, GLfloat);

        bool leaf();

    public:

        patch(point *, const double *,
              point *, const double *,
              point *, const double *, double, double, double);
        ~patch();

        patch *get_child(int i) { return C[i]; }
        point *get_point(int i) { return P[i]; }

        void seed(geonrm&, geopos&, geotex&, const double *,
                                             const double *, int);
        patch *step(context&, app::frustum_v&,
                    const double *, const double *, double, int, GLsizei&);
        void   draw(context&, GLsizei, GLsizei, GLsizei);
        void   view(GLsizei, const GLfloat *);
//      void   prep(GLsizei);
//      void   wire();
    };

    //-------------------------------------------------------------------------

    class context
    {
        // Invariant: N[i] is neighbor n[i] of P.

        patch *P;
        patch *N[3];
        int    n[3];
        int    d;

        patch *get_loc_L(int);
        patch *get_loc_R(int);

    public:

        context(patch *, patch *, int,
                         patch *, int,
                         patch *, int);
        context(context&, int);

        patch *get_patch(   );
        patch *get_child(int);
        patch *get_local(int);
        point *get_point(int);

        int get_depth() const { return d; }
    };

    //-------------------------------------------------------------------------
}

#endif
