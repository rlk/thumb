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

#ifndef GEOMAP
#define GEOMAP

#include <list>

#include "app-frustum.hpp"
#include "ogl-program.hpp"
#include "ogl-buffer.hpp"
#include "ogl-image.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------
    // Geomap page

    class page
    {
        int    d;
        int    i;
        int    j;
        double n[3];
        double a;
        double r;
        double f;

        bool live;

        page *P[4];

    public:

        page(int, int, int, int, int, int, double, double, double, double);
       ~page();

        bool view(app::frustum_v&, double, double);
        void draw(                 double, double);

        int get_d() const { return d; }
        int get_i() const { return i; }
        int get_j() const { return j; }

        page *child(int c) { return P[c]; }

        double angle(const double *, double);

        bool get_live() const { return live;  }
        void set_dead()       { live = false; }
    };

    //-------------------------------------------------------------------------
    // Geomap

    class geocsh;
    class geomap
    {
        std::string pattern;

        double ext_W;
        double ext_E;
        double ext_S;
        double ext_N;
        double r0;
        double r1;
        int    w;
        int    h;
        int    c;
        int    b;
        int    s;
        int    S;
        int    D;
        int mip_w;
        int mip_h;
        bool  lsb;
        page   *P;

        bool dirty;

        const ogl::program *prog;

        geocsh      *cache;
        GLushort    *image;
        ogl::buffer *buffer;
        ogl::image  *index;

        GLushort& index_x(int, int, int);
        GLushort& index_y(int, int, int);
        GLushort& index_l(int, int, int);

        void do_index(int, int, int, GLushort, GLushort, GLushort);
        void do_eject(int, int, int, GLushort, GLushort, GLushort,
                                     GLushort, GLushort, GLushort);

    public:

        geomap(geocsh *, std::string, double, double);
       ~geomap();

        void cache_page(const page *, int, int);
        void eject_page(const page *, int, int);

        void seed(const double *, double, double);
        void proc();
        void draw(double, const double *) const;

        std::string name(const page *);

        page *root() { return P;   }
        bool  swab() { return lsb; }
    };

    typedef std::list<geomap *>           geomap_l;
    typedef std::list<geomap *>::iterator geomap_i;
}

//-----------------------------------------------------------------------------

#endif
