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

// Construct a new page with associated texture object. If this is a REAL new
// page (with a loading time), fill the texture with transparency to mask the
// asynchronous PBO process.

sph_page::sph_page(int f, int i, GLuint o, int t) : sph_item(f, i), o(o), t(t)
{
    static const GLfloat p[4] = { 0, 0, 0, 0 };
        
    if (t)
    {
        glBindTexture(GL_TEXTURE_2D, o);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, p);
    }
}

// Construct a load task. Map the PBO to provide a destination for the loader.

sph_task::sph_task(int f, int i, GLuint u, GLsizei s) : sph_item(f, i), u(u)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
    {
        glBufferData(GL_PIXEL_UNPACK_BUFFER, s, 0, GL_STREAM_DRAW);
        p = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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

// Construct a file table entry. Load the TIFF briefly to determine its format.
    
sph_file::sph_file(const std::string& name) : name(name)
{
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

void sph_set::insert(sph_page page, int t)
{
    m[page] = t;
}

void sph_set::remove(sph_page page)
{
    m.erase(page);
}

GLuint sph_set::search(sph_page page, int t, int &n)
{
    std::map<sph_page, int>::iterator i = m.find(page);
    
    GLuint o = 0;
    
    if (i != m.end())
    {
        n = i->first.t;
        o = i->first.o;
        m[page] = t;
    }
    return o;
}

GLuint sph_set::eject(int t, int i)
{
    assert(!m.empty());
    
    // Determine the lowest priority and least- and most-recently used pages.
    
    std::map<sph_page, int>::iterator a = m.begin();
    std::map<sph_page, int>::iterator z = m.begin();
    std::map<sph_page, int>::iterator l = m.begin();
    std::map<sph_page, int>::iterator e;
    
    for (e = m.begin(); e != m.end(); l = e, ++e)
    {
        if (e->second < a->second) a = e;
        if (e->second > z->second) z = e;
    }

    // If the LRU page was not used in this frame or the last, eject it.
    // Otherwise consider the lowest-priority loaded page and eject it has
    // lower priority than the incoming page.

    if (a->second < t - 1)
    {
        GLuint o = a->first.o;
        m.erase(a);
        return o;
    }
    if (i < l->first.i)
    {
        GLuint o = l->first.o;
        m.erase(l);
        return o;
    }
    return 0;
}

void sph_set::draw()
{
    int l = log2(size);
    int r = (l & 1) ? (1 << ((l - 1) / 2)) : (1 << (l / 2));
    int c = (l & 1) ? (1 << ((l + 1) / 2)) : (1 << (l / 2));

    glEnable(GL_TEXTURE_2D);

    glPushAttrib(GL_VIEWPORT_BIT);
    {
        glViewport(0, 0, c * 32, r * 32);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(c, 0, r, 0, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        std::map<sph_page, int>::iterator i = m.begin();

        for     (int y = 0; y < r; ++y)
            for (int x = 0; x < c; ++x)

                if (i != m.end())
                {
                    glBindTexture(GL_TEXTURE_2D, i->first.o);
                    glBegin(GL_QUADS);
                    {
                        glTexCoord2f(0.f, 0.f); glVertex2i(x,     y);
                        glTexCoord2f(1.f, 0.f); glVertex2i(x + 1, y);
                        glTexCoord2f(1.f, 1.f); glVertex2i(x + 1, y + 1);
                        glTexCoord2f(0.f, 1.f); glVertex2i(x,     y + 1);
                    }
                    glEnd();
                    i++;
                }
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------

sph_cache::sph_cache(int n) : needs(32), loads(8), pages(n), waits(n)
{
    GLuint b;
//    GLuint o;
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
    /*
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
    */
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
    
    needs.insert(sph_task(-1, -1));
    needs.insert(sph_task(-2, -2));
    needs.insert(sph_task(-3, -3));
    needs.insert(sph_task(-4, -4));
    
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
/*    
    while (!texs.empty())
    {
        glDeleteBuffers(1, &texs.back());
        texs.pop_back();
    }
*/    
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

GLuint sph_cache::get_page(int f, int i, int t, int& n)
{
    GLuint o;
    
    // If this page has loaded data, return it.
    
    if ((o = pages.search(sph_page(f, i), t, n)))
        return o;

    // Else if this page is waiting as filler, return it.

    if ((o = waits.search(sph_page(f, i), t, n)))
        return o;

    // Else request the data and let the page wait as filler.

    if (!needs.full() && !pbos.empty())
    {
        waits.insert(sph_page(f, i, filler, 0), t);
        needs.insert(sph_task(f, i, pbos.deq(), pagelen(f)));
    }

    n = 0;
    return filler;
}

// Handle incoming textures on the loads queue.

void sph_cache::update(int t)
{
    for (int c = 0; !loads.empty() && c < 4; ++c)
    {
        // Take the next load task and eject a loaded page to make room.
        
        sph_task task = loads.remove();
        GLuint o;
    
        waits.remove(sph_page(task.f, task.i));
        
        if (pages.full())
            o = pages.eject(t, task.i);
        else
            glGenTextures(1, &o);
            
        if (o)
        {
            pages.insert(sph_page(task.f, task.i, o, t), t);
            
            task.make_texture(o, files[task.f].w, files[task.f].h,
                                 files[task.f].c, files[task.f].b);
        }
        else task.dump_texture();

        pbos.enq(task.u);
    }
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

