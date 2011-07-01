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

#ifndef SPH_CACHE_HPP
#define SPH_CACHE_HPP

#include <vector>
#include <string>
#include <list>

#include <GL/glew.h>
#include <tiffio.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "tree.hpp"
#include "queue.hpp"

//------------------------------------------------------------------------------

template <typename T> class fifo : public std::list <T>
{
public:

    void enq(T& p) {
        this->push_front(p);
    }
    
    T deq() {
        T p = this->front();
        this->pop_front();
        return p;
    }
};

//------------------------------------------------------------------------------

struct sph_file
{
    sph_file(const std::string& name);
    
    std::string name;
    uint32 w, h;
    uint16 c, b;
};

//------------------------------------------------------------------------------

struct sph_page
{
    sph_page(int=-1, int=-1, GLuint=0, int=0);

    int    f;
    int    i;
    GLuint o;
    int    t;
    
    bool operator<(const sph_page& that) const {
        if (f == that.f)
            return i < that.i;
        else
            return f < that.f;
    }
};

//------------------------------------------------------------------------------

struct sph_task
{
    sph_task(int=-1, int=-1, GLuint=0, GLsizei=0);
    
    int    f;
    int    i;
    GLuint u;
    void  *p;
    
    void make_texture(GLuint, uint32, uint32, uint16, uint16);
    void load_texture(TIFF *, uint32, uint32, uint16, uint16);
    void dump_texture();

    bool operator<(const sph_task& that) const {
        if (f == that.f)
            return i < that.i;
        else
            return f < that.f;
    }
};

//------------------------------------------------------------------------------

class sph_cache
{
public:

    sph_cache(int);
   ~sph_cache();

    int    add_file(const std::string&);
    GLuint get_page(int, int, int, int&);
    GLuint get_fill() { return filler; }
    
    void update(int);
    void draw();

private:

    std::vector<sph_file> files;

    fifo<GLuint> pbos;
    fifo<GLuint> texs;
        
    tree <sph_page> pages;
    tree <sph_page> waits;
    queue<sph_task> needs;
    queue<sph_task> loads;

    GLuint filler;
    const int size;

    GLsizei pagelen(int f);
    
    SDL_Thread *thread[4];
    
    friend int loader(void *);
};

//------------------------------------------------------------------------------

#endif
