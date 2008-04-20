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

#ifndef GEOCSH
#define GEOCSH

#include <SDL.h>
#include <SDL_thread.h>
#include <png.h>

#include <list>

#include "app-glob.hpp"
#include "uni-geomap.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------
    // Geometry data buffer

    class buffer
    {
        png_uint_32 w;
        png_uint_32 h;
        png_byte    c;
        png_byte    b;

        GLubyte   *dat;
        png_bytep *row;

        bool ret;

    public:

        buffer(int, int, int, int);
       ~buffer();

        buffer *load(std::string);

        const GLvoid *data() const { return dat; }
        bool          stat() const { return ret; }
    };

    //-------------------------------------------------------------------------
    // Geometry data cache

    class geocsh
    {
        static const GLenum form[2][4];
        static const GLenum type[2];

        typedef std::list<buffer *> buff_list;

        // Needed page

        struct need
        {
            geomap *M;
            page   *P;
            need(geomap *M=0, page *P=0) : M(M), P(P) { }
        };
        typedef std::multimap<double, need, std::greater<double> > need_map;

        // Loaded page

        struct load
        {
            geomap *M;
            page   *P;
            buffer *B;
            load(geomap *M=0, page *P=0, buffer *B=0) : M(M), P(P), B(B) { }
        };
        typedef std::list<load> load_list;

        // Cache index

        struct line
        {
            geomap *M;
            int     x;
            int     y;
            line(geomap *M=0, int x=0, int y=0) : M(M), x(x), y(y) { }
        };
        typedef std::map<page *, line> line_map;

        int c;
        int b;
        int s;
        int S;
        int w;
        int h;
        int n;
        int m;
        int count;

        bool run;
        bool debug;

        ogl::image  *cache;

        std::list<page *> cache_lru;
        line_map          cache_idx;

        need_map  needs;
        load_list loads;
        buff_list buffs;

        SDL_cond   *need_cond;
        SDL_mutex  *need_mutex;
        SDL_mutex  *load_mutex;
        SDL_mutex  *buff_mutex;

        std::vector<SDL_Thread *> load_thread;

        void proc_needs(const double *, double, double, app::frustum_v&);
        void proc_loads();

    public:

        geocsh(int, int, int, int, int);
       ~geocsh();

        void init();
        void seed(const double *, double, double, geomap&);
        void proc(const double *, double, double, app::frustum_v&);

        void bind(GLenum) const;
        void free(GLenum) const;

        void draw() const;
        void wire(double, double) const;

        int pool_w() const { return w * S; }
        int pool_h() const { return h * S; }

        // Data access service API.

        bool get_needed(geomap **, page **, buffer **);
        void put_loaded(geomap *,  page *,  buffer *);
    };
}

//-----------------------------------------------------------------------------

#endif
