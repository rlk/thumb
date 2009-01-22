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

#ifndef UNI_GEOMAP_HPP
#define UNI_GEOMAP_HPP

#include <SDL.h>
#include <SDL_thread.h>

#include <list>
#include <set>

#include "app-frustum.hpp"
#include "ogl-program.hpp"
#include "ogl-buffer.hpp"
#include "ogl-image.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------
    // Geomap page

    class page;

    typedef page *page_p;

    enum page_state_t
    {
        page_default,
        page_needed,
        page_loading,
        page_cached,
        page_missing
    };

    class page
    {
        page_state_t st;        // State

        int     d;              // Depth (0 is base)
        int     c;              // Depth (0 is root)
        int     i;              // Row
        int     j;              // Column
        double  f;              // Data fraction (page area / data area)
        double  a;              // Cap angle
        double  n[3];           // Cap normal
        page_p  P[4];           // Child pointers

        int  visible_cache_serial;
        bool visible_cache_result;

    public:

        page(int, int, int, int, int, int, int, double, double, double, double);
       ~page();

        void view(app::frustum_v&, double, double, int, int);
        void draw(                 double, double);

        int get_d() const { return d; }
        int get_i() const { return i; }
        int get_j() const { return j; }

        page *child(int c) { return P[c]; }

        double angle(const double *, double);

        bool visible(app::frustum_v&, double, double, int);
        bool needed (app::frustum_v&, double, double, int, int, double);

        page_state_t get_state() const          { return st; }
        void         set_state(page_state_t nst) { st = nst; }
    };

    typedef std::set<page_p>           page_s;
    typedef std::set<page_p>::iterator page_i;

    //-------------------------------------------------------------------------
    // Geomap

    class geocsh;
    class geomap
    {
        std::string pattern;    // Data file name format

        double ext_W;           // Cylindrical map extents
        double ext_E;
        double ext_S;
        double ext_N;
        double r0;              // Planet's minimum radius
        double r1;              // Planet's maximum radius
        double cutoff;          // Pixels/texel resolution cutoff

        int    w;               // Total map width  in pixels
        int    h;               // Total map height in pixels
        int    c;               // Channel count
        int    b;               // Bytes per channel
        int    s;               // Page size, border not included
        int    S;               // Page size, border included
        int    D;               // Mipmap pyramid depth
        int    mip_w;           // Mipmap index image width  (2 x base)
        int    mip_h;           // Mipmap index image height (2 x base)
        bool   dirty;           // Mipmap index image is dirty?
        bool   lsb;             // 16-bit data is little-endian?

        page_p  root;           // Page hierarchy root

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

        void view_page(page *, app::frustum_v&, double, double, int);

    public:

        geomap(geocsh *, std::string, double, double);
       ~geomap();

        void cache_page(const page *, int, int);
        void eject_page(const page *, int, int);

        void view(app::frustum_v&, double, double, int);
        void proc();
        void draw(double, const double *) const;

        std::string name(const page *);

        bool   swab()   const { return lsb; }
        double get_r0() const { return r0;  }
        double get_r1() const { return r1;  }
    };

    typedef std::list<geomap *>           geomap_l;
    typedef std::list<geomap *>::iterator geomap_i;
}

//-----------------------------------------------------------------------------

#endif
