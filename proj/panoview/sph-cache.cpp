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
}

sph_cache::~sph_cache()
{
}

//------------------------------------------------------------------------------

int sph_cache::add_file(const char *name)
{
//    if (TIFF *T = TIFFOpen(name, "r"))
//    {
//        int c = int(files.size());
//
//        files.push_back(T);
//
//        return c;
//    }
    return 0;
}

//------------------------------------------------------------------------------

// Select an OpenGL internal texture format for an image with c channels and
// b bytes per channel.

static GLenum internal_form(uint16 c, uint16 b)
{
    if (b == 8)
        switch (c)
        {
        case  1: return GL_LUMINANCE;
        case  2: return GL_LUMINANCE_ALPHA;
        case  3: return GL_RGB;
        default: return GL_RGBA;
        }
    if (b == 16)
        switch (c)
        {
        case  1: return GL_LUMINANCE16;
        case  2: return GL_LUMINANCE16_ALPHA16;
        case  3: return GL_RGB16;
        default: return GL_RGBA16;
        }
    if (b == 32)
        switch (c)
        {
        case  1: return GL_R32F;
        case  2: return GL_RG32F;
        case  3: return GL_RGB32F;
        default: return GL_RGBA32F;
        }
        
    return 0;
}

// Select an OpenGL external texture format for an image with c channels.

static GLenum external_form(uint16 c)
{
    switch (c)
    {
    case  1: return GL_LUMINANCE;
    case  2: return GL_LUMINANCE_ALPHA;
    case  3: return GL_RGB;
    default: return GL_RGBA;
    }
}

// Select an OpenGL data type for an image with b bytes per channel.

static GLenum external_type(uint16 b)
{
    if (b ==  8) return GL_UNSIGNED_BYTE;
    if (b == 16) return GL_UNSIGNED_SHORT;
    if (b == 32) return GL_FLOAT;

    return 0;
}

//------------------------------------------------------------------------------

// Remove the least recently-used page and release its OpenGL texture object.

void sph_cache::rem_page()
{
//    if (!used.empty())
//    {
//        page_map::iterator it = pages.find(used.back());
//
//        glDeleteTextures(1, &it->second);
//
//        used.pop_back();
//    }
}

// Return the texture object associated with the requested page. Load the
// image if necessary. Eject the LRU page as needed.

GLuint sph_cache::get_page(int f, int i)
{
//    page_map::iterator it = pages.find(id(f, i));
//    
//    if (it == pages.end())
//    {
//        if (up(f, i))
//        {
//            if (GLuint o = new_page(files[f]))
//            {
//                if (pages.size() >= size)
//                    rem_page();
//
//                pages[id(f, i)] = o;
//                
//                return o;
//            }
//        }
//        return 0;
//    }
//    else return it->second;
}

GLuint sph_cache::new_page(TIFF *T)
{
    // Determine the parameters of the incoming image.
    
    uint32 w;
    uint32 h;
    uint16 b;
    uint16 c;
    uint32 s = (uint32) TIFFScanlineSize(T);
    
    TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &w);
    TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &h);
    TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &b);
    TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &c);

    GLenum in   = internal_form(c, b);
    GLenum ex   = external_form(c);
    GLenum type = external_type(b);
        
    // Load the pixel data and initialize an OpenGL texture object.

    GLuint texture = 0;
    uint8 *pixels  = 0;
    
    if ((pixels = (uint8 *) malloc(h * s)))
    {
        for (uint32 r = 0; r < h; ++r)
            TIFFReadScanline(T, (uint8 *) pixels + r * s, r, 0);
            
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
        glTexImage2D(GL_TEXTURE_2D, 0, in, w, h, 1, ex, type, pixels);
    }
    return texture;
}

// Seek upward to the root of the page tree and choose the appropriate base
// image. Navigate to the requested sub-image directory on the way back down.
#if 0
int sph_cache::up(int f, int i)
{
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
#endif
//------------------------------------------------------------------------------
