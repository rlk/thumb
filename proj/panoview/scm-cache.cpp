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

scm_cache::scm_cache(int s, int n, int c, int b, float r0, float r1) :
    pages(),
    waits(),
    needs(need_queue_size),
    loads(load_queue_size),
    texture(0),
    next(1),
    n(n),
    c(c),
    b(b),
    r0(r0),
    r1(r1)
{
    // Launch the image loader threads.

    int loader(void *data);

    thread[0] = SDL_CreateThread(loader, this);
    thread[1] = SDL_CreateThread(loader, this);
    thread[2] = SDL_CreateThread(loader, this);
    thread[3] = SDL_CreateThread(loader, this);

    // Generate pixel buffer objects.

    for (int i = 0; i < 64; ++i)
    {
        GLuint b;
        glGenBuffers(1, &b);
        pbos.push_back(b);
    }

    // Limit the cache size to the maximum array texture depth.

    GLint max;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max);

    size = std::min(max, s);

    // Generate the array texture object.

    GLenum i = scm_internal_form(c, b, 0);
    GLenum e = scm_external_form(c, b, 0);
    GLenum t = scm_external_type(c, b, 0);

    glGenTextures  (1, &texture);
    glBindTexture  (GL_TEXTURE_2D_ARRAY,  texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_CLAMP);
    glTexImage3D   (GL_TEXTURE_2D_ARRAY, 0, i, n, n, size, 1, e, t, 0);
}

scm_cache::~scm_cache()
{
    // Continue servicing the loads queue until the needs queue is emptied.

    sync(0);

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

    // Release the texture.

    glDeleteTextures(1, &texture);
}

//------------------------------------------------------------------------------

int scm_cache::add_file(const std::string& name)
{
    int f = -1;

    // Try to load the named file.

    if (scm_file *n = new scm_file(name))
    {
        // If succesful, add it to the collection.

        f = int(files.size());
        files.push_back(n);
    }
    return f;
}

// Return the layer for the requested page. Request the page if necessary.

int scm_cache::get_page(int f, long long i, int t, int& n)
{
    // If this page is waiting, return the filler.

    scm_page wait = waits.search(scm_page(f, i), t);

    if (wait.valid())
    {
        n    = wait.t;
        return wait.l;
    }

    // If this page is loaded, return the layer.

    scm_page page = pages.search(scm_page(f, i), t);

    if (page.valid())
    {
        n    = page.t;
        return page.l;
    }

    // If this page does not exist, return the filler.

    uint64 o = files[f]->offset(i);

    if (o == 0)
        return 0;

    // Otherwise request the page and add it to the waiting set.

    if (!needs.full() && !pbos.empty())
    {
        needs.insert(scm_task(f, i, o, pbos.deq(), files[f]->length()));
        waits.insert(scm_page(f, i, 0), t);
    }

    n = 0;
    return 0;
}

// Handle incoming textures on the loads queue.

void scm_cache::update(int t)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glPushAttrib(GL_PIXEL_MODE_BIT);
    {
        for (int c = 0; !loads.empty() && c < max_loads_per_update; ++c)
        {
            scm_task task = loads.remove();
            scm_page page(task.f, task.i);

            waits.remove(page);

            if (task.d)
            {
                if (next < size)
                    page.l = next;
                else
                {
                    scm_page victim = pages.eject(t, page.i);

                    if (victim.valid())
                        page.l = victim.l;
                }

                if (page.l >= 0)
                {
                    page.t = t;
                    pages.insert(page, t);

                    task.make_page(page.l, files[task.f]->get_w(),
                                           files[task.f]->get_h(),
                                           files[task.f]->get_c(),
                                           files[task.f]->get_b(),
                                           files[task.f]->get_g());
                }
                else task.dump_page();
            }
            else task.dump_page();

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

void scm_cache::sync(int t)
{
    while (!needs.empty())
        update(t);
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

            t0 = t0 * (r1 - r0) + r0;
            t1 = t1 * (r1 - r0) + r0;

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

                task.load_page(T, w, h, c, b, g);
                task.d = true;
            }
            TIFFClose(T);
        }
        cache->loads.insert(task);
    }
    return 0;
}

//------------------------------------------------------------------------------

