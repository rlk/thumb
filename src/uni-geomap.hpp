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

#include "app-frustum.hpp"
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
/*
        double W;
        double E;
        double S;
        double N;
*/
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

    class geomap
    {
        std::string pattern;

        double ext_W;
        double ext_E;
        double ext_S;
        double ext_N;
        int    w;
        int    h;
        int    c;
        int    b;
        int    s;
        int    S;
        int    D;
        int mip_w;
        int mip_h;
        double r0;
        double r1;
        page   *P;

        bool dirty;

        GLushort   *image;
        ogl::image *index;

        GLushort& index_x(int, int, int);
        GLushort& index_y(int, int, int);
        GLushort& index_l(int, int, int);

        void do_index(int, int, int, GLushort, GLushort, GLushort);
        void do_eject(int, int, int, GLushort, GLushort, GLushort,
                                     GLushort, GLushort, GLushort);

    public:

        geomap(std::string, double, double);
       ~geomap();

        void cache_page(const page *, int, int);
        void eject_page(const page *, int, int);

        void init(int, int) const;
        void proc();
        void draw() const;

        std::string name(const page *);

        page *root() { return P; }
    };
}

//-----------------------------------------------------------------------------

#endif
