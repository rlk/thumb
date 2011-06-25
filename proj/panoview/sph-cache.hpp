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

#include <GL/glew.h>
#include <tiffio.h>

#include "tree.hpp"
#include "queue.hpp"

//------------------------------------------------------------------------------

struct sph_task
{
    sph_task(int f=-1, int i=-1, void *p=0) : f(f), i(i), p(p) { }
    
    int    f;
    int    i;
    void  *p;
};

struct sph_page
{
    sph_page(int f=-1, int i=-1, GLuint o=0) : f(f), i(i), o(o) { }

    int    f;
    int    i;
    GLuint o;
    
    bool operator<(const sph_page& that) const {
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

    int    add_file(const char *);
    GLuint get_page(int, int);
     
private:

    GLuint new_page(TIFF *);
    void   rem_page();
    
    int up(int, int);
    int dn(int, int);

    // Cache data structure.

    tree<sph_page> pages;

    queue<sph_task> req;
    queue<sph_task> rep;
    
    const int size;

};

//------------------------------------------------------------------------------

#endif