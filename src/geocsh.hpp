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

#include <list>
#include <queue>

#include "glob.hpp"
#include "geomap.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------
    // Loaded page queue

    class loaded_queue
    {
        struct load;

        std::queue<load> Q;

        SDL_mutex *mutex;

    public:

        loaded_queue();
       ~loaded_queue();

        void enqueue(geomap *,  page *,  unsigned char *);
        bool dequeue(geomap **, page **, unsigned char **);
    };

    //-------------------------------------------------------------------------
    // Needed page queue

    class needed_queue
    {
        struct need;

        std::list<need> Q;

        SDL_mutex *mutex;
        SDL_sem   *sem;
        bool       run;

    public:

        needed_queue();
       ~needed_queue();

        void enqueue(loaded_queue *,  geomap *,  page *);
        bool dequeue(loaded_queue **, geomap **, page **);

        void stop();
    };

    //-------------------------------------------------------------------------
    // Geometry data cache

    class geocsh
    {
        struct index_line
        {
            geomap *M;
            page   *P;
            double  k;
        };
        struct cache_line
        {
            geomap *M;
            int     x;
            int     y;
        };

        int c;
        int b;
        int s;
        int w;
        int h;
        int n;
        int m;

        index_line *index;
        ogl::image *cache;

        std::list<page *>             cache_lru;
        std::map <page *, cache_line> cache_map;

        needed_queue *need_Q;
        loaded_queue *load_Q;
        SDL_Thread   *loader;

    public:

        geocsh(int, int, int, int, int);
       ~geocsh();

        void init();
        void seed(const double *, double, double, geomap&);
        void proc(const double *, double, double, app::frustum_v&);

        void bind(GLenum unit) { cache->bind(unit); }
        void free(GLenum unit) { cache->free(unit); }
    };
}

//-----------------------------------------------------------------------------

#endif
