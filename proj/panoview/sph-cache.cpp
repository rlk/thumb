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

sph_cache::sph_cache(int n) : needs(32), loads(8), size(n)
{
    int loader(void *data);

    thread[0] = SDL_CreateThread(loader, this);
    thread[1] = SDL_CreateThread(loader, this);
    thread[2] = SDL_CreateThread(loader, this);
    thread[3] = SDL_CreateThread(loader, this);
 
    GLuint o;

    for (int i = 0; i < 64; ++i)
    {
        glGenBuffers(1, &o);
        pbos.push_back(o);
    }
}

sph_cache::~sph_cache()
{
    // Continue servicing the loads queue until the needs queue is emptied.
    
    while (!needs.empty())
        update(0);
    
    // Enqueue an exit command for each loader thread.
    
    needs.insert(sph_task());
    needs.insert(sph_task());
    needs.insert(sph_task());
    needs.insert(sph_task());
    
    // Await their exit. 
    
    int s;

    SDL_WaitThread(thread[0], &s);
    SDL_WaitThread(thread[1], &s);
    SDL_WaitThread(thread[2], &s);
    SDL_WaitThread(thread[3], &s);
    
    while (!pbos.empty())
    {
        glDeleteBuffers(1, &pbos.back());
        pbos.pop_back();
    }
}

//------------------------------------------------------------------------------

// Append a string to the file list and return its index.

int sph_cache::add_file(const std::string& name)
{
    int f = int(files.size());

    files.push_back(sph_file(name));

    return f;
}

// Return the texture object associated with the requested page. Request the
// image if necessary. Eject the LRU page if needed.

GLuint sph_cache::get_page(int t, int f, int i)
{
    if (pages.size())
    {
        sph_page& page = pages.search(sph_page(f, i), t);
    
        if (page.f == f && page.i == i)
            return page.o;
    }

    if (!pbos.empty() && !needs.full())
    {
        uint32 w = files[f].w;
        uint32 h = files[f].h;
        uint16 c = files[f].c;
        uint16 b = files[f].b;

        GLsizei s;

        if (c == 3 && b == 8)
            s = w * h * 4 * b / 8;
        else
            s = w * h * c * b / 8;

        while (pages.size() >= size)
        {
            sph_page page = pages.eject();
            glDeleteTextures(1, &page.o);
        }

        GLuint o = pbos.front();
        pbos.pop_front();

        needs.insert(sph_task(f, i, o, s));
        pages.insert(sph_page(f, i), t);
    }

    return 0;
}

// Handle incoming textures on the loads queue.

void sph_cache::update(int t)
{
    for (int c = 0; !loads.empty() && c < 4; ++c)
    {
        sph_task  task = loads.remove();
        sph_page& page = pages.search(sph_page(task.f, task.i), t);
        
        if (page.f == task.f && page.i == task.i)

            page.o = task.make_texture(files[task.f].w, files[task.f].h,
                                       files[task.f].c, files[task.f].b);

        else task.cancel();

        pbos.push_back(task.o);
    }
}

//------------------------------------------------------------------------------

struct draw_state
{
    int r, c;
    int i, j;
};

static void draw_page(sph_page& page, void *data)
{
    draw_state *st = (draw_state *) data;

    glBindTexture(GL_TEXTURE_2D, page.o);

    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.f, 0.f); glVertex2i(st->j,     st->i);
        glTexCoord2f(1.f, 0.f); glVertex2i(st->j + 1, st->i);
        glTexCoord2f(1.f, 1.f); glVertex2i(st->j + 1, st->i + 1);
        glTexCoord2f(0.f, 1.f); glVertex2i(st->j,     st->i + 1);
    }
    glEnd();

    if (++st->j >= st->c)
    {
        st->i += 1;
        st->j  = 0;
    }
}

void sph_cache::draw()
{
    int l = log2(size);

    draw_state st;

    st.r = (l & 1) ? (1 << ((l - 1) / 2)) : (1 << (l / 2));
    st.c = (l & 1) ? (1 << ((l + 1) / 2)) : (1 << (l / 2));
    st.i = 0;
    st.j = 0;

    glEnable(GL_TEXTURE_2D);

    glPushAttrib(GL_VIEWPORT_BIT);
    {
        glViewport(0, 0, st.c * 32, st.r * 32);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(st.c, 0, st.r, 0, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        pages.map(draw_page, &st);
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------

// Select an OpenGL internal texture format for an image with c channels and
// b bits per channel.

static GLenum internal_form(uint16 c, uint16 b)
{
    if (b == 8)
        switch (c)
        {
        case  1: return GL_LUMINANCE8;
        case  2: return GL_LUMINANCE_ALPHA;
        case  3: return GL_RGBA8; // *
        default: return GL_RGBA8;
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
    case  3: return GL_BGRA; // *
    default: return GL_BGRA;
    }
}

// Select an OpenGL data type for an image with c channels of b bits.

static GLenum external_type(uint16 c, uint16 b)
{
    if (b ==  8 && c == 3) return GL_UNSIGNED_INT_8_8_8_8_REV; // *
    if (b ==  8 && c == 4) return GL_UNSIGNED_INT_8_8_8_8_REV;

    if (b ==  8) return GL_UNSIGNED_BYTE;
    if (b == 16) return GL_UNSIGNED_SHORT;
    if (b == 32) return GL_FLOAT;

    return 0;
}

// * 24-bit images are always padded to 32 bits.

//------------------------------------------------------------------------------

sph_task::sph_task(int f, int i, GLuint o, GLsizei s) : f(f), i(i), o(o), p(0)
{
    if (o)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, o);
        {
            glBufferData(GL_PIXEL_UNPACK_BUFFER, s, 0, GL_STREAM_DRAW);
            p = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
}

// Upload the pixel buffer to a newly-generated OpenGL texture object.

GLuint sph_task::make_texture(uint32 w, uint32 h, uint16 c, uint16 b)
{
    GLuint t = 0;

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, o);
    {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

        GLenum in = internal_form(c, b);
        GLenum ex = external_form(c);
        GLenum ty = external_type(c, b);
        
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
        
        glTexImage2D(GL_TEXTURE_2D, 0, in, w, h, 1, ex, ty, 0);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    
    return t;
}

// A texture was loaded but is no longer necessary. Discard the pixel buffer.

void sph_task::cancel()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, o);
    {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// Load the current TIFF directory into a newly-allocated pixel buffer.

void sph_task::load_texture(TIFF *T, uint32 w, uint32 h, uint16 c, uint16 b)
{
    // Confirm the page format.
    
    uint32 W, H;
    uint16 C, B;
    
    TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &W);
    TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &H);
    TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &B);
    TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &C);

    if (W == w && H == h && B == b && C == c)
    {
        // Pad a 24-bit image to 32-bit BGRA.
        
        if (c == 3 && b == 8)
        {
            if (void *q = malloc(TIFFScanlineSize(T)))
            {
                const uint32 S = w * 4 * b / 8;

                for (uint32 r = 0; r < h; ++r)
                {
                    TIFFReadScanline(T, q, r, 0);
                    
                    for (int j = w - 1; j >= 0; --j)
                    {
                        uint8 *s = (uint8 *) q         + j * c * b / 8;
                        uint8 *d = (uint8 *) p + r * S + j * 4 * b / 8;
                        
                        d[0] = s[2];
                        d[1] = s[1];
                        d[2] = s[0];
                        d[3] = 0xFF;
                    }
                }
                free(q);
            }
        }
        else
        {
            const uint32 S = TIFFScanlineSize(T);

            for (uint32 r = 0; r < h; ++r)
                TIFFReadScanline(T, (uint8 *) p + r * S, r, 0);
        }
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

        if (TIFF *T = TIFFOpen(cache->files[task.f].name.c_str(), "r"))
        {
            if (up(T, task.i))
            {
                uint32 w = cache->files[task.f].w;
                uint32 h = cache->files[task.f].h;
                uint16 c = cache->files[task.f].c;
                uint16 b = cache->files[task.f].b;
            
                task.load_texture(T, w, h, c, b);
                cache->loads.insert(task);
            }
            TIFFClose(T);
        }        
    
    return 0;
}

//------------------------------------------------------------------------------

sph_file::sph_file(const std::string& name) : name(name)
{
    // Load the TIFF briefly to determine its format.
    
    if (TIFF *T = TIFFOpen(name.c_str(), "r"))
    {
        TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &w);
        TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &h);
        TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &b);
        TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &c);

        TIFFClose(T);
    }
}

//------------------------------------------------------------------------------

