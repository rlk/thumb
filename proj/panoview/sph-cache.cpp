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

#include <cstdlib>

#include "sph-cache.hpp"
#include "cube.hpp"

//------------------------------------------------------------------------------

sph_cache::sph_cache(int n) : size(n)
{
    glGenTextures(1, &texture);
    
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

sph_cache::~sph_cache()
{
    for (file_vec::iterator f = files.begin(); f != files.end(); ++f)
        TIFFClose(*f);
}

//------------------------------------------------------------------------------

void indent(int d)
{
        for (int i = 0; i < d; ++i)
            printf("  ");
}

void test(TIFF *T, int d)
{
    uint32 *v;
    uint16  n;
    
    if (TIFFGetField(T, TIFFTAG_SUBIFD, &n, &v))
    {
        uint32 v0 = v[0];
        uint32 v1 = v[1];
        uint32 v2 = v[2];
        uint32 v3 = v[3];

        if (v0)
        {
            indent(d);
            printf("%x\n", v0);
            TIFFSetSubDirectory(T, v0);
            test(T, d + 1);
        }
        if (v1)
        {
            indent(d);
            printf("%x\n", v1);
            TIFFSetSubDirectory(T, v1);
            test(T, d + 1);
        }
        if (v2)
        {
            indent(d);
            printf("%x\n", v2);
            TIFFSetSubDirectory(T, v2);
            test(T, d + 1);
        }
        if (v3)
        {
            indent(d);
            printf("%x\n", v3);
            TIFFSetSubDirectory(T, v3);
            test(T, d + 1);
        }
    }
}

int sph_cache::add_file(const char *name)
{
    if (TIFF *T = TIFFOpen(name, "r"))
    {
        int c = int(files.size());

//        test(T, 0);

        files.push_back(T);

        return c;
    }
    return 0;
}

//------------------------------------------------------------------------------

GLuint sph_cache::get_page(int f, int i)
{
    TIFF *T = files[f];
        
    if (TIFFSetDirectory(T, i)) //up(f, i))
    {
        uint32 w, h, s = (uint32) TIFFScanlineSize(T);
        uint16 b, c;
        
        TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &w);
        TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &h);
        TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &b);
        TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &c);
        
        if (uint8 *p = (uint8 *) malloc(h * s))
        {
            for (uint32 r = 0; r < h; ++r)
                TIFFReadScanline(T, p + r * s, r, 0);
                
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 1,
                         GL_RGB, GL_UNSIGNED_BYTE, p);
            
            free(p);
            
            return texture;
        }
    }
    return 0;
}

int sph_cache::up(int f, int i)
{
    return 1;

    if (i < 6)
        return TIFFSetDirectory(files[f], i);
    else
    {
        if (up(f, face_parent(i)))
            return dn(f, face_index(i));
        else
            return 0;
    }
}

int sph_cache::dn(int f, int i)
{
    uint32 *v;
    uint16  n;
    
    if (TIFFGetField(files[f], TIFFTAG_SUBIFD, &n, &v))
    {
        if (n > 0 && v[i] > 0)
            return TIFFSetSubDirectory(files[f], v[i]);
    }
    return 0;
}

//------------------------------------------------------------------------------
