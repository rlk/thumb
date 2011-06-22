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

//------------------------------------------------------------------------------

class sph_cache
{
public:

    sph_cache(int);
   ~sph_cache();

    int    add_file(const char *);
    GLuint get_page(int, int);
     
private:

    struct page
    {        
        bool operator()(page& a, page& b) { return (a.c < b.c || a.p < b.p); }
        int c;
        int p;
    };

    typedef std::vector<TIFF *>       file_vec;
    typedef std::map   <page, GLuint> page_map;
    typedef std::list  <page>         page_list;

    file_vec  files;
    page_map  pages;
    page_list used;
    int       size;

    GLuint texture;

    int up(int, int);
    int dn(int, int);
};

//------------------------------------------------------------------------------

#endif