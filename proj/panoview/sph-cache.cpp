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
    GLuint b;
    GLuint o;
    int    i;
    
    // Launch the image loader threads.
    
    int loader(void *data);

    thread[0] = SDL_CreateThread(loader, this);
    thread[1] = SDL_CreateThread(loader, this);
    thread[2] = SDL_CreateThread(loader, this);
    thread[3] = SDL_CreateThread(loader, this);

    // Generate pixel buffer objects.
 
    for (i = 0; i < 64; ++i)
    {
        glGenBuffers(1, &b);
        pbos.push_back(b);
    }
    
    // Generate texture objects.
    
    for (i = 0; i < n; ++i)
    {
        glGenTextures(1, &o);
        glBindTexture  (GL_TEXTURE_2D, o);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
        texs.push_back(o);
    }
    
    // Initialize the default transparent texture.
    
    GLfloat p[4] = { 0, 0, 0, 0 };
    
    glGenTextures(1, &filler);
    glBindTexture  (GL_TEXTURE_2D, filler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, p);
}

sph_cache::~sph_cache()
{
    // Continue servicing the loads queue until the needs queue is emptied.
    
    while (!needs.empty())
        update(0);
    
    // Enqueue an exit command for each loader thread.
    
    needs.insert(sph_task(-1, 1));
    needs.insert(sph_task(-1, 2));
    needs.insert(sph_task(-1, 3));
    needs.insert(sph_task(-1, 4));
    
    // Await their exit. 
    
    int s;

    SDL_WaitThread(thread[0], &s);
    SDL_WaitThread(thread[1], &s);
    SDL_WaitThread(thread[2], &s);
    SDL_WaitThread(thread[3], &s);

    // Release the pixel buffer objects.
    
    while (!pbos.empty())
    {
        glDeleteBuffers(1, &pbos.back());
        pbos.pop_back();
    }
    
    // Release the texture objects.
    
    while (!texs.empty())
    {
        glDeleteBuffers(1, &texs.back());
        texs.pop_back();
    }
    
    glDeleteTextures(1, &filler);
}

//------------------------------------------------------------------------------

// Append a string to the file list and return its index. Queue the roots.

int sph_cache::add_file(const std::string& name)
{
    int f = int(files.size());

    files.push_back(sph_file(name));

    return f;
}

// Return the texture object associated with the requested page. Request the
// image if necessary.

GLuint sph_cache::get_page(int f, int i, int t, int& time)
{
    // If this page is loaded, return it.
    
    if (pages.size())
    {
        sph_page& page = pages.search(sph_page(f, i), t);
    
        if (page.f == f && page.i == i)
        {
            time = page.t;
            return page.o;
        }
    }

    // Else if this page is waiting, return filler.

    if (waits.size())
    {
        sph_page& page = waits.search(sph_page(f, i), t);
    
        if (page.f == f && page.i == i)
            return filler;
    }

    // Else request the page and note it waiting.

    if (!needs.full() && !pbos.empty())
    {
        waits.insert(sph_page(f, i), t);
        needs.insert(sph_task(f, i, pbos.deq(), pagelen(f)));
    }

    return filler;
}

// Handle incoming textures on the loads queue.

void sph_cache::update(int t)
{
    for (int c = 0; !loads.empty() && c < 4; ++c)
    {
        // Eject a loaded page to make room.
        
        while (pages.size() >= size)
        {
            sph_page page = pages.eject();
            texs.enq(page.o);
        }

        // Use the next load task to create a working page.

        sph_task task = loads.remove();

        GLuint o = texs.deq();

        pages.insert(sph_page(task.f, task.i, o, t), t);
        waits.remove(sph_page(task.f, task.i));        
        
        task.make_texture(o, files[task.f].w, files[task.f].h,
                             files[task.f].c, files[task.f].b);
        pbos.enq(task.u);        
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

// Compute the length of a page buffer for file f.

GLsizei sph_cache::pagelen(int f)
{
    uint32 w = files[f].w;
    uint32 h = files[f].h;
    uint16 c = files[f].c;
    uint16 b = files[f].b;

    if (c == 3 && b == 8)
        return w * h * 4 * b / 8; // *
    else
        return w * h * c * b / 8;
}

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

sph_page::sph_page(int f, int i, GLuint o, int t) : f(f), i(i), o(o), t(t)
{
    static const GLfloat p[4] = { 0, 0, 0, 0 };
        
    if (o)
    {
        glBindTexture(GL_TEXTURE_2D, o);
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, p);
    }
}

sph_task::sph_task(int f, int i, GLuint u, GLsizei s) : f(f), i(i), u(u), p(0)
{
    if (u)
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
        {
            glBufferData(GL_PIXEL_UNPACK_BUFFER, s, 0, GL_STREAM_DRAW);
            p = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }
}

// Upload the pixel buffer to a newly-generated OpenGL texture object.

void sph_task::make_texture(GLuint o, uint32 w, uint32 h, uint16 c, uint16 b)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
    {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

        glBindTexture(GL_TEXTURE_2D, o);
        glTexImage2D (GL_TEXTURE_2D, 0, internal_form(c, b), w, h, 1,
                      external_form(c), external_type(c, b), 0);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// A texture was loaded but is no longer necessary. Discard the pixel buffer.

void sph_task::dump_texture()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
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
    {
        assert(task.u);
        assert(task.p);
        
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

