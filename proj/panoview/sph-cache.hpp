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

#include <vector>
#include <list>
#include <map>

#include "splay.hpp"

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

    struct id
    {
        id(int f, int i) : f(f), i(i) { }
        int f;
        int i;
        bool operator<(const id& that) const { return (this->f < that.f
                                                    || this->i < that.i); }
    };

    typedef std::vector<TIFF *>     file_vec;
    typedef std::map   <id, GLuint> page_map;
    typedef std::list  <id>         page_list;

    file_vec  files;
    page_map  pages;
    page_list used;
    const int size;

    // I/O buffer.

    void  *bufptr;
    size_t buflen;
};

//------------------------------------------------------------------------------

#endif