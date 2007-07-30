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

#include <SDL.h>
#include <SDL_thread.h>

#include <list>
#include <queue>

#include "texture.hpp"
#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class geomap;
    class page;

    class texture_pool
    {
        GLuint *pool;
        bool   *stat;

        int n;
        int s;
        int c;
        int b;

    public:

        texture_pool(int, int, int, int);
       ~texture_pool();

        GLuint get();
        void   put(GLuint);
    };

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

        void enqueue(page *,  unsigned char *);
        bool dequeue(page **, unsigned char **);
    };

    //-------------------------------------------------------------------------
    // Needed page queue

    class needed_queue
    {
        struct need;

        std::queue<need> Q;

        SDL_mutex *mutex;
        SDL_sem   *sem;
        bool       run;

    public:

        needed_queue();
       ~needed_queue();

        void enqueue(std::string&, loaded_queue *,  page *);
        bool dequeue(std::string&, loaded_queue **, page **);

        void stop();
    };

    //-------------------------------------------------------------------------
    // Geomap page

    class page
    {
        enum { dead_state,
               wait_state,
               live_state,
               skip_state } state;

        page  *P[4], *U;
        double b[6];
        double c[3];
        double area;
        int d, i, j;
        double L;
        double R;
        double B;
        double T;

        GLuint object;
        int    hint;

        bool test(const double *);
        bool value(const double *);
        bool visible(const double *,
                     const double *);

        void volume() const;

        bool is_visible;
        bool is_valued;

    public:

        static int count;

        page(page *, int, int, int, int, int, int,
             double, double, double, double, double, double);
       ~page();

        void   assign(GLuint);
        GLuint remove();
        void   ignore();

//      void prep(const double *, const double *, bool, bool);
        void prep(const double *, int, int);
        void draw(geomap&, int, int, int, int);
        void wire();

        int lod() const { return d; }
    };

    //-------------------------------------------------------------------------
    // Geomap

    class geomap
    {
        std::string name;

        bool debug;
        int c;
        int b;
        int s;
        int d;
        double k;
        double r0;
        double r1;

        GLfloat hoff;
        GLfloat hscl;

        page *P;

        std::list<page *> LRU;

        static SDL_Thread   *loader;
        static needed_queue *need_Q;
               loaded_queue *load_Q;
               texture_pool *text_P;

    public:

        static void init();
        static void fini();

        geomap(std::string, int, int, int, int, int, 
               double, double, double, double, double, double, double);
       ~geomap();

        bool needed(page *, int, int, int);
        bool loaded();

        void purge();
        void used(page *);

        void draw(const double *,
                  const double *, int=0, int=0);
        void wire();

        void set_debug(bool b) { debug = b; }
    };
}

//-----------------------------------------------------------------------------

#endif
