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
#include <cassert>
#include <limits>

#include "scm-cache.hpp"
#include "cube.hpp"

//------------------------------------------------------------------------------

// Zero the given texture object by uploading a single black pixel.

static void clear(GLuint t)
{
    static const GLfloat p[] = { 0, 0, 0, 0 };

    glBindTexture   (GL_TEXTURE_2D, t);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, p);
    glTexImage2D    (GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, p);
}

// Select an OpenGL internal texture format for an image with c channels and
// b bits per channel.

static GLenum internal_form(uint16 c, uint16 b, uint16 g)
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
        case  1: return GL_LUMINANCE32F_ARB;
        case  2: return GL_LUMINANCE_ALPHA32F_ARB;
        case  3: return GL_RGB32F_ARB;
        default: return GL_RGBA32F_ARB;
        }

    return 0;
}

// Select an OpenGL external texture format for an image with c channels.

static GLenum external_form(uint16 c, uint16 b, uint16 g)
{
    if (b == 8)
        switch (c)
        {
        case  1: return GL_LUMINANCE;
        case  2: return GL_LUMINANCE_ALPHA;
        case  3: return GL_BGRA; // *
        default: return GL_BGRA;
        }
    else
        switch (c)
        {
        case  1: return GL_LUMINANCE;
        case  2: return GL_LUMINANCE_ALPHA;
        case  3: return GL_RGB;
        default: return GL_RGBA;
        }
}

// Select an OpenGL data type for an image with c channels of b bits.

static GLenum external_type(uint16 c, uint16 b, uint16 g)
{
    if (b ==  8 && c == 3) return GL_UNSIGNED_INT_8_8_8_8_REV; // *
    if (b ==  8 && c == 4) return GL_UNSIGNED_INT_8_8_8_8_REV;

    if (b ==  8) return /*(g == 2) ? GL_BYTE  :*/ GL_UNSIGNED_BYTE;
    if (b == 16) return /*(g == 2) ? GL_SHORT :*/ GL_UNSIGNED_SHORT;
    if (b == 32) return GL_FLOAT;

    return 0;
}

// * 24-bit images are always padded to 32 bits.

//------------------------------------------------------------------------------

// Construct a load task. Map the PBO to provide a destination for the loader.

scm_task::scm_task(int f, int i, uint64 o, GLuint u, GLsizei s)
    : scm_item(f, i), o(o), u(u), d(false)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
    {
        glBufferData(GL_PIXEL_UNPACK_BUFFER, s, 0, GL_STREAM_DRAW);
        p = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// Upload the pixel buffer to the OpenGL texture object.

void scm_task::make_texture(GLuint o, uint32 w, uint32 h,
                                      uint16 c, uint16 b, uint16 g)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
    {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

        glBindTexture  (GL_TEXTURE_2D, o);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);
        glTexImage2D   (GL_TEXTURE_2D, 0, internal_form(c, b, g), w, h, 1,
                                          external_form(c, b, g),
                                          external_type(c, b, g), 0);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// A texture was loaded but is no longer necessary. Discard the pixel buffer.

void scm_task::dump_texture()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, u);
    {
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

// Load the current TIFF directory image data into the mapped pixel buffer.

void scm_task::load_texture(TIFF *T, uint32 w, uint32 h,
                                     uint16 c, uint16 b, uint16 g)
{
    // Confirm the page format.

    uint32 W, H;
    uint16 C, B, G;

    TIFFGetField(T, TIFFTAG_IMAGEWIDTH,      &W);
    TIFFGetField(T, TIFFTAG_IMAGELENGTH,     &H);
    TIFFGetField(T, TIFFTAG_BITSPERSAMPLE,   &B);
    TIFFGetField(T, TIFFTAG_SAMPLESPERPIXEL, &C);
    TIFFGetField(T, TIFFTAG_SAMPLEFORMAT,    &G);

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
            const uint32 S = (uint32) TIFFScanlineSize(T);

            for (uint32 r = 0; r < h; ++r)
                TIFFReadScanline(T, (uint8 *) p + r * S, r, 0);
        }
    }
}

//------------------------------------------------------------------------------

scm_set::~scm_set()
{
    std::map<scm_page, int>::iterator i;

    for (i = m.begin(); i != m.end(); ++i)
        glDeleteTextures(1, &i->first.o);
}

void scm_set::insert(scm_page page, int t)
{
    m[page] = t;
}

void scm_set::remove(scm_page page)
{
    m.erase(page);
}

// Search for the given page in this page set. If found, update the page entry
// with the current time t to indicate recent use.

scm_page scm_set::search(scm_page page, int t)
{
    std::map<scm_page, int>::iterator i = m.find(page);

    if (i != m.end())
    {
        scm_page p = i->first;

        remove(p);
        insert(p, t);
        return(p);
    }
    return scm_page();
}

// Eject a page from this set to accommodate the addition of a new page.
// t is the current cache time and i is the index of the incoming page.
// The general polity is LRU, but with considerations for time and priority
// that help mitigate thrashing.

scm_page scm_set::eject(int t, int i)
{
    assert(!m.empty());

    // Determine the lowest priority and least-recently used pages.

    std::map<scm_page, int>::iterator a = m.end();
    std::map<scm_page, int>::iterator l = m.end();
    std::map<scm_page, int>::iterator e;

    for (e = m.begin(); e != m.end(); ++e)
    {
        if (a == m.end() || e->second < a->second) a = e;
                                                   l = e;
    }

    // If the LRU page was not used in this frame or the last, eject it.
    // Otherwise consider the lowest-priority loaded page and eject if it
    // has lower priority than the incoming page.

    if (a != m.end() && a->second < t - 2)
    {
        scm_page page = a->first;
        m.erase(a);
        return page;
    }
    if (l != m.end() && i < l->first.i)
    {
        scm_page page = l->first;
        m.erase(l);
        return page;
    }
    return scm_page();
}

void scm_set::draw()
{
    int l = log2(size);
    int r = (l & 1) ? (1 << ((l - 1) / 2)) : (1 << (l / 2));
    int c = size / r;

    glUseProgram(0);

    glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glViewport(0, 0, c * 32, r * 32);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(c, 0, r, 0, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor4f(1.f, 1.f, 1.f, 1.f);

        std::map<scm_page, int>::iterator i = m.begin();

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

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------

scm_cache::scm_cache(int n) :
    pages(n),
    waits(n),
    needs("need", need_queue_size),
    loads("load", load_queue_size),
    size(0)
{
    GLuint b;
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

    // Initialize the default filler texture.

    glGenTextures(1, &filler);
    clear(filler);
}

scm_cache::~scm_cache()
{
    // Continue servicing the loads queue until the needs queue is emptied.

    while (!needs.empty())
        update(0);

    // Enqueue an exit command for each loader thread.

    needs.insert(scm_task(-1, -1));
    needs.insert(scm_task(-2, -2));
    needs.insert(scm_task(-3, -3));
    needs.insert(scm_task(-4, -4));

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

    // Release the file index / offset maps.

    std::vector<scm_file *>::iterator i;

    for (i = files.begin(); i != files.end(); ++i)
        delete (*i);

    // Release the default texture.

    glDeleteTextures(1, &filler);
}

//------------------------------------------------------------------------------
#if 0
static void debug_on(int l)
{
    static const GLfloat color[][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
    };
    glPixelTransferf(GL_RED_SCALE,   color[l][0]);
    glPixelTransferf(GL_GREEN_SCALE, color[l][1]);
    glPixelTransferf(GL_BLUE_SCALE,  color[l][2]);
}
#endif
//------------------------------------------------------------------------------

// Append a string to the file list and return its index. Queue the roots.

int scm_cache::add_file(const std::string& name, float r0, float r1)
{
    int f = int(files.size());

    files.push_back(new scm_file(name, r0, r1));

    return f;
}

// Return the texture object associated with the requested page. Request the
// image if necessary.

GLuint scm_cache::get_page(int f, int i, int t, int& n)
{
    // If this page is waiting, return the filler.

    scm_page wait = waits.search(scm_page(f, i), t);

    if (wait.valid())
    {
        n    = wait.t;
        return wait.o;
    }

    // If this page is loaded, return the texture.

    scm_page page = pages.search(scm_page(f, i), t);

    if (page.valid())
    {
        n    = page.t;
        return page.o;
    }

    // If this page does not exist, return the filler.

    uint64 o = files[f]->offset(i);

    if (o == 0)
        return filler;

    // Otherwise request the page and add it to the waiting set.

    if (!needs.full() && !pbos.empty())
    {
        needs.insert(scm_task(f, i, o, pbos.deq(), pagelen(f)));
        waits.insert(scm_page(f, i, filler), t);
    }

    n = 0;
    return filler;
}

// Handle incoming textures on the loads queue.

void scm_cache::update(int t)
{
    glPushAttrib(GL_PIXEL_MODE_BIT);
    {
        for (int c = 0; !loads.empty() && c < max_loads_per_update; ++c)
        {
            scm_task task = loads.remove();
            scm_page page(task.f, task.i);

            waits.remove(page);

            if (task.d)
            {
                if (pages.full())
                {
                    scm_page victim = pages.eject(t, page.i);

                    if (victim.valid())
                    {
                        size  -= pagelen(victim.f);
                        page.o =         victim.o;
                    }
                }
                else
                    glGenTextures(1, &page.o);

                if (page.o)
                {
                    page.t = t;
                    pages.insert(page, t);

                    task.make_texture(page.o, files[task.f]->get_w(),
                                              files[task.f]->get_h(),
                                              files[task.f]->get_c(),
                                              files[task.f]->get_b(),
                                              files[task.f]->get_g());
                    size += pagelen(page.f);
                }
                else task.dump_texture();
            }
            else task.dump_texture();

            pbos.enq(task.u);
        }
    }
    glPopAttrib();
}

void scm_cache::flush()
{
    while (!pages.empty())
        pages.eject(0, -1);
}

void scm_cache::draw()
{
    pages.draw();
}

//------------------------------------------------------------------------------

void scm_cache::page_bounds(int i, const int *vv, int vc, float& r0, float& r1)
{
    r0 =  std::numeric_limits<float>::max();
    r1 = -std::numeric_limits<float>::max();

    for (int vi = 0; vi < vc; ++vi)
    {
        float t0;
        float t1;

        files[vv[vi]]->bounds(i, t0, t1);

        r0 = std::min(r0, t0);
        r1 = std::max(r1, t1);
    }
}

bool scm_cache::page_status(int i, const int *vv, int vc,
                                   const int *fv, int fc)
{
    for (int vi = 0; vi < vc; ++vi)
        if (files[vv[vi]]->status(i))
            return true;

    for (int fi = 0; fi < fc; ++fi)
        if (files[fv[fi]]->status(i))
            return true;

    return false;
}

//------------------------------------------------------------------------------

// Compute the length of a page buffer for file f.
// TODO: eliminate

GLsizei scm_cache::pagelen(int f)
{
    return files[f]->length();
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
    uint64 *v;
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
    scm_cache *cache = (scm_cache *) data;
    scm_task   task;

    while ((task = cache->needs.remove()).f >= 0)
    {
        assert(task.valid());

        if (TIFF *T = TIFFOpen(cache->files[task.f]->get_name(), "r"))
        {
            if (TIFFSetSubDirectory(T, task.o))
            {
                uint32 w = cache->files[task.f]->get_w();
                uint32 h = cache->files[task.f]->get_h();
                uint16 c = cache->files[task.f]->get_c();
                uint16 b = cache->files[task.f]->get_b();
                uint16 g = cache->files[task.f]->get_g();

                task.load_texture(T, w, h, c, b, g);
                task.d = true;
            }
            TIFFClose(T);
        }
        cache->loads.insert(task);
    }
    return 0;
}

//------------------------------------------------------------------------------

