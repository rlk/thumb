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
#include <map>

#include <GL/glew.h>
#include <tiffio.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "queue.hpp"

#include "scm-file.hpp"

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

struct scm_item
{
    scm_item()                   : f(-1), i(-1) { }
    scm_item(int f, long long i) : f( f), i( i) { }

    int       f;
    long long i;

    bool valid() const { return (f >= 0 && i >= 0); }

    bool operator<(const scm_item& that) const {
        if (i == that.i)
            return f < that.f;
        else
            return i < that.i;
    }
};

//------------------------------------------------------------------------------

struct scm_page : public scm_item
{
    scm_page()                                    : scm_item(    ), o(0), t(0) { }
    scm_page(int f, long long i)                  : scm_item(f, i), o(0), t(0) { }
    scm_page(int f, long long i, GLuint o)        : scm_item(f, i), o(o), t(0) { }
    scm_page(int f, long long i, GLuint o, int t) : scm_item(f, i), o(o), t(t) { }

    GLuint o;
    int    t;
};

//------------------------------------------------------------------------------

struct scm_task : public scm_item
{
    scm_task()                   : scm_item(    ), u(0), d(false), p(0) { }
    scm_task(int f, long long i) : scm_item(i, f), u(0), d(false), p(0) { }
    scm_task(int f, long long i, uint64 o, GLuint u, GLsizei s);

    uint64 o;
    GLuint u;
    bool   d;
    void  *p;

    void make_texture(GLuint, uint32, uint32, uint16, uint16, uint16);
    void load_texture(TIFF *, uint32, uint32, uint16, uint16, uint16);
    void dump_texture();
};

//------------------------------------------------------------------------------

class scm_set
{
public:

    scm_set(int size) : size(size) { }
   ~scm_set();

    int  count() const { return int(m.size()); }
    bool full()  const { return (count() >= size); }
    bool empty() const { return (m.empty()); }

    scm_page search(scm_page, int);
    void     insert(scm_page, int);
    void     remove(scm_page);

    scm_page eject(int, long long);
    void     draw();

private:

    std::map<scm_page, int> m;

    int size;
};

//------------------------------------------------------------------------------

class scm_cache
{
public:

    scm_cache(int);
   ~scm_cache();

    int    add_file(const std::string&, float, float, int);
    GLuint get_page(int, long long, int, int&);
    GLuint get_fill() { return filler; }

    int    get_size() const { return size; }
    float  get_r0()   const { return r0;   }
    float  get_r1()   const { return r1;   }

    // TODO: Use friend to encapsulate this.

    void page_bounds(long long, const int *, int, float&, float&);
    bool page_status(long long, const int *, int,
                                const int *, int);

    void update(int);
    void flush();
    void draw();

    void set_debug(bool b) { debug = b; }

private:

    static const int need_queue_size      = 32;   // 32
    static const int load_queue_size      =  8;   //  8
    static const int max_loads_per_update =  4;   //  2

    std::vector<scm_file *> files;

    scm_set pages;
    scm_set waits;

    queue<scm_task> needs;
    queue<scm_task> loads;

    fifo<GLuint> pbos;

    GLsizei pagelen(int);
    GLuint  filler;
    bool    debug;
    int     size;
    float   r0;
    float   r1;

    SDL_Thread *thread[4];

    friend int loader(void *);
};

//------------------------------------------------------------------------------

#endif
