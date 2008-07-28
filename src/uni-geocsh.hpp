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
#include <set>

#include "app-glob.hpp"
#include "uni-geomap.hpp"
#include "ogl-buffer.hpp"

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
        GLubyte   *ptr;

        ogl::buffer pbo;

        bool ret;

        void swab1us(GLubyte *, const GLubyte *) const;
        void mask1us(GLubyte *, const GLubyte *) const;
        void swab3ub(GLubyte *, const GLubyte *) const;
        void mask3ub(GLubyte *, const GLubyte *) const;

    public:

        buffer(int, int, int, int);
       ~buffer();

        buffer *load(std::string, bool);

        const GLvoid *data() const { return dat; }
        bool          stat() const { return ret; }

        void bind() {                   pbo.bind(); }
        void free() {                   pbo.free(); }
        void zero() {                   pbo.zero(); }
        void wmap() { ptr = (GLubyte *) pbo.wmap(); }
        void umap() { ptr = 0;          pbo.umap(); }
    };

    //-------------------------------------------------------------------------
    // Geometry data cache

    class geocsh
    {
        static GLenum internal_format(int, int);
        static GLenum external_format(int, int);
        static GLenum      pixel_type(int, int);

        typedef std::list<buffer *> buff_list;

        // Needed page

        struct need
        {
            geomap *M;
            page   *P;
            need(geomap *M=0, page *P=0) : M(M), P(P) { }

            bool operator<(const need& that) const {
                return (P->get_d() < that.P->get_d() ||
                        P->get_i() < that.P->get_i() ||
                        P->get_j() < that.P->get_j() || P < that.P);
            }
        };
        typedef std::multimap<double, need, std::greater<double> > need_map;
        typedef std::set<need>                                     need_set;

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

        need_map  seeds;
        need_set  needs;
        load_list loads;
        buff_list buffs;
        buff_list waits;

        SDL_sem   *need_sem;
        SDL_sem   *buff_sem;

        SDL_mutex *seed_mutex;
        SDL_mutex *need_mutex;
        SDL_mutex *load_mutex;
        SDL_mutex *buff_mutex;

        std::vector<SDL_Thread *> load_thread;

        void proc_waits();
        void proc_loads();
        void proc_needs(const double *, double, double, app::frustum_v&);

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
        bool get_loaded(geomap **, page **, buffer **);
        void put_loaded(geomap *,  page *,  buffer *);
    };

    typedef std::list<geocsh *>           geocsh_l;
    typedef std::list<geocsh *>::iterator geocsh_i;
}

//-----------------------------------------------------------------------------

#endif
