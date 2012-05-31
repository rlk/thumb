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
#include "scm-index.hpp"

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


//------------------------------------------------------------------------------

scm_cache::scm_cache(int n) :
    pages(n),
    waits(n),
    needs("need", need_queue_size),
    loads("load", load_queue_size),
    size(0),
    r0(+std::numeric_limits<float>::max()),
    r1(-std::numeric_limits<float>::max())
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

// Append a string to the file list and return its index. Cache the bounds.

int scm_cache::add_file(const std::string& name, float n0, float n1, int dd)
{
    int f = int(files.size());

    files.push_back(new scm_file(name, n0, n1, dd));

    r0 = std::min(r0, n0);
    r1 = std::max(r1, n1);

    return f;
}

// Return the texture object associated with the requested page. Request the
// image if necessary.

GLuint scm_cache::get_page(int f, long long i, int t, int& n)
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

void scm_cache::page_bounds(long long i, const int *vv, int vc, float& s0, float& s1)
{
    if (vc > 0)
    {
        s0 =  std::numeric_limits<float>::max();
        s1 = -std::numeric_limits<float>::max();

        for (int vi = 0; vi < vc; ++vi)
        {
            float t0;
            float t1;

            files[vv[vi]]->bounds(uint64(i), t0, t1);

            s0 = std::min(s0, t0);
            s1 = std::max(s1, t1);
        }
    }
    else
    {
        s0 = 1.0;
        s1 = 1.0;
    }
}

bool scm_cache::page_status(long long i, const int *vv, int vc,
                                         const int *fv, int fc)
{
    for (int vi = 0; vi < vc; ++vi)
        if (files[vv[vi]]->status(uint64(i)))
            return true;

    for (int fi = 0; fi < fc; ++fi)
        if (files[fv[fi]]->status(uint64(i)))
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

