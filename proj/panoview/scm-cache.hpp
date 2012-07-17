//  Copyright (C) 2005-2011 Robert Kooima
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

#ifndef SCM_CACHE_HPP
#define SCM_CACHE_HPP

#include <vector>
#include <string>
#include <list>

#include <GL/glew.h>
#include <tiffio.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "queue.hpp"

#include "scm-file.hpp"
#include "scm-task.hpp"
#include "scm-set.hpp"

//------------------------------------------------------------------------------

template <typename T> class fifo : public std::list <T>
{
public:

    void enq(T p) {
        this->push_back(p);
    }

    T deq() {
        T p = this->front();
        this->pop_front();
        return p;
    }
};

//------------------------------------------------------------------------------

class scm_cache
{
public:

    scm_cache(int, int, int, int, float, float);
   ~scm_cache();

    int    add_file(const std::string&);
    int    get_page(int, long long, int, int&);
    float  get_r0() const { return r0; }
    float  get_r1() const { return r1; }

    // TODO: Use friend to encapsulate this.

    void page_bounds(long long, const int *, int, float&, float&);
    bool page_status(long long, const int *, int,
                                const int *, int);

    void update(int);  // Cycle the cache once
    void sync  (int);  // Cycle the cache until all needs are served

    void flush();
    void draw();
    void bind() const { glBindTexture(GL_TEXTURE_2D_ARRAY, texture); }

private:

    static const int need_queue_size      = 32;   // 32
    static const int load_queue_size      =  8;   //  8
    static const int max_loads_per_update =  4;   //  2

    std::vector<scm_file *> files; // SCM TIFF data files
    scm_set pages;                 // Page set currently in cache
    scm_set waits;                 // Page set currently being loaded

    queue<scm_task> needs;         // Page loader thread input  queue
    queue<scm_task> loads;         // Page loader thread output queue

    fifo<GLuint> pbos;             // Asynchronous upload ring

    GLuint  texture;
    int     next;
    int     size;
    int     n;
    int     c;
    int     b;
    float   r0;
    float   r1;

    SDL_Thread *thread[4];         // Page loader threads
    friend int  loader(void *);    // Page loader function
};

typedef std::vector<scm_cache *>           scm_cache_v;
typedef std::vector<scm_cache *>::iterator scm_cache_i;

//------------------------------------------------------------------------------

#endif
