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

sph_cache::sph_cache(int n) : needs(n), loads(16), size(n)
{
    int loader(void *data);

    thread[0] = SDL_CreateThread(loader, this);
    thread[1] = SDL_CreateThread(loader, this);
}

sph_cache::~sph_cache()
{
    int s;
    
    needs.insert(sph_task());
    needs.insert(sph_task());
    
    SDL_WaitThread(thread[0], &s);
    SDL_WaitThread(thread[1], &s);
}

//------------------------------------------------------------------------------

// Append a string to the file list and return its index.

int sph_cache::add_file(const std::string& name)
{
    int f = int(files.size());

    files.push_back(name);

    return f;
}

// Return the texture object associated with the requested page. Request the
// image if necessary. Eject the LRU page if needed.

GLuint sph_cache::get_page(int f, int i)
{
    sph_page page = pages.search(sph_page(f, i, 0));
    
    if (page.f == f && page.i == i)
        return page.o;
    else
    {
        if (pages.size() == size)
        {
            page = pages.eject();
            glDeleteTextures(1, &page.o);
        }
            
        needs.insert(sph_task(f, i, 0));
        pages.insert(sph_page(f, i, 0));
        return 0;
    }
}

// Handle any incoming textures on the loads queue.

void sph_cache::update()
{
    while (loads.count() > 0)
    {
        sph_task task = loads.remove();
        sph_page page = pages.search(sph_page(task.f, task.i, 0));
        
        if (page.f == task.f && page.i == task.i)
        {
            pages.remove(page);
            page.o = task.make_texture();
            pages.insert(page);
        }
    }
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

// Upload the pixel buffer to a newly-generated OpenGL texture object.

GLuint sph_task::make_texture()
{
    GLenum in = internal_form(c, b);
    GLenum ex = external_form(c);
    GLenum ty = external_type(b);
        
    GLuint texture;
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glTexImage2D(GL_TEXTURE_2D, 0, in, w, h, 1, ex, ty, p);
    
    free(p);
    
    return texture;
}

// Load the current TIFF directory into a newly-allocated pixel buffer.

void sph_task::load_texture(TIFF *T)
{
    uint32 s = (uint32) TIFFScanlineSize(T);
    
    TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &w);
    TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &h);
    TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &b);
    TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &c);

    if ((p = malloc(h * s)))
    {
        for (uint32 r = 0; r < h; ++r)
            TIFFReadScanline(T, (uint8 *) p + r * s, r, 0);
    }
}

//------------------------------------------------------------------------------

static int up(TIFF *T, int i);
static int dn(TIFF *T, int i);

// Seek upward to the root of the page tree and choose the appropriate base
// image. Navigate to the requested sub-image directory on the way back down.

static int up(TIFF *T, int i)
{
    if (i < 6)
        return TIFFSetDirectory(T, i);
    else
    {
        if (up(T, face_parent(i)))
            return dn(T, face_index(i));
        else
            return 0;
    }
}

static int dn(TIFF *T, int i)
{
    uint32 *v;
    uint16  n;
    
    if (TIFFGetField(T, TIFFTAG_SUBIFD, &n, &v))
    {
        if (n > 0 && v[i] > 0)
            return TIFFSetSubDirectory(T, v[i]);
    }
    return 0;
}

// Load textures. Remove a task from the cache's needed queue, open and read
// the TIFF image file, and insert the task in the cache's loaded queue. Exit
// when given a negative file index.

int loader(void *data)
{
    sph_cache *cache = (sph_cache *) data;
    sph_task   task;
    
    while ((task = cache->needs.remove()).f >= 0)

        if (TIFF *T = TIFFOpen(cache->files[task.f].c_str(), "r"))
        {
            if (up(T, task.i))
            {
                task.load_texture(T);
                cache->loads.insert(task);
            }
            TIFFClose(T);
        }        
    
    return 0;
}

//------------------------------------------------------------------------------

