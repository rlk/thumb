//  Copyright (C) 2007 Robert Kooima
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

#include <stdexcept>
#include <iostream>
#include <png.h>

#include "geocsh.hpp"
#include "conf.hpp"

//=============================================================================

bool load_png(png_bytepp bp, std::string name)
{
    png_structp rp = 0;
    png_infop   ip = 0;
    FILE       *fp = 0;

    bool rr = false;

    // Initialize all PNG import data structures.

    if ((fp = fopen(name.c_str(), "rb")))
    {
        if ((rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        {
            if ((ip = png_create_info_struct(rp)))
            {
                // Enable the default PNG error handler.

                if (setjmp(png_jmpbuf(rp)) == 0)
                {
                    // Read the PNG.

                    png_init_io   (rp, fp);
                    png_read_info (rp, ip);
                    png_read_image(rp, bp);

                    rr = true;
                }

                png_destroy_read_struct(&rp, &ip, 0);
            }
        }
        fclose(fp);
    }
    return rr;
}

//=============================================================================

uni::buffer_pool::buffer_pool(int w, int h, int c, int b) :
    w(w), h(h), c(c), b(b), N(0)
{
    mutex = SDL_CreateMutex();
}

uni::buffer_pool::~buffer_pool()
{
    if (mutex) SDL_DestroyMutex(mutex);

    // Free all buffers.

    while (!avail.empty())
    {
        if (avail.front().rp) free(avail.front().rp);
        if (avail.front().pp) free(avail.front().pp);
        
        avail.pop_front();
    }
}

uni::buffer_pool::buff uni::buffer_pool::get()
{
    buff B;

    SDL_mutexP(mutex);
    {
        if (avail.empty())
        {
            // No buffers available.  Allocate and initialize a new one.

            B.pp = (GLubyte   *) malloc(h * w * c * b);
            B.rp = (png_bytep *) malloc(h * sizeof (png_bytep));

            for (int i = 0; i < h; ++i)
                B.rp[h - i - 1] = B.pp + i * w * c * b;

            N++;

            printf("pool size = %d\n", N);
        }
        else
        {
            // Buffers are available.  Return one.

            B = avail.front();
            avail.pop_front();
        }
    }
    SDL_mutexV(mutex);

    return B;
}

void uni::buffer_pool::put(buff B)
{
    // Add the given buffer to the available list.

    SDL_mutexP(mutex);
    {
        avail.push_front(B);
    }
    SDL_mutexV(mutex);
}

//=============================================================================

struct uni::loaded_queue::load
{
    geomap           *M;
    page             *P;
    buffer_pool::buff b;

    load(geomap *M, page *P, buffer_pool::buff b) : M(M), P(P), b(b) { }
};

//-----------------------------------------------------------------------------

uni::loaded_queue::loaded_queue()
{
    mutex = SDL_CreateMutex();
}

uni::loaded_queue::~loaded_queue()
{
    if (mutex) SDL_DestroyMutex(mutex);
}

bool uni::loaded_queue::find(const page *P)
{
    bool b = false;

    SDL_mutexP(mutex);
    {
        for (std::list<load>::iterator i = Q.begin(); i != Q.end(); ++i)
            if (i->P == P)
            {
                b = true;
                break;
            }
    }
    SDL_mutexV(mutex);

    return b;
}

void uni::loaded_queue::enqueue(geomap *M, page *P, buffer_pool::buff b)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    Q.push_back(load(M, P, b));
    SDL_mutexV(mutex);
}

bool uni::loaded_queue::dequeue(geomap **M, page **P, buffer_pool::buff *b)
{
    bool rr = false;

    // If the queue is not empty, dequeue.

    SDL_mutexP(mutex);
    {
        if (!Q.empty())
        {
            *M = Q.front().M;
            *P = Q.front().P;
            *b = Q.front().b;

            Q.pop_front();

            rr = true;
        }
    }
    SDL_mutexV(mutex);

    return rr;
}

//=============================================================================

struct uni::needed_queue::need
{
    geomap *M;
    page   *P;

    need(geomap *M, page *P) : M(M), P(P) { }
};

//-----------------------------------------------------------------------------

uni::needed_queue::needed_queue() : run(true)
{
    mutex = SDL_CreateMutex();
    sem   = SDL_CreateSemaphore(0);
}

uni::needed_queue::~needed_queue()
{
    if (sem)   SDL_DestroySemaphore(sem);
    if (mutex) SDL_DestroyMutex(mutex);
}

bool uni::needed_queue::find(const page *P)
{
    bool b = false;

    SDL_mutexP(mutex);
    {
        for (std::list<need>::iterator i = Q.begin(); i != Q.end(); ++i)
            if (i->P == P)
            {
                b = true;
                break;
            }
    }
    SDL_mutexV(mutex);

    return b;
}

void uni::needed_queue::enqueue(geomap *M, page *P)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    {
        Q.push_back(need(M, P));
    }
    SDL_mutexV(mutex);

    // Signal the consumer.

    SDL_SemPost(sem);
}

bool uni::needed_queue::dequeue(geomap **M, page **P)
{
    bool b;

    // Wait for the signal to proceed.

    SDL_SemWait(sem);

    // Dequeue the request.

    SDL_mutexP(mutex);
    {
        if ((b = run))
        {
            *M = Q.front().M;
            *P = Q.front().P;

            Q.pop_front();
        }
    }
    SDL_mutexV(mutex);

    return b;
}

void uni::needed_queue::stop()
{
    // Clear the run flag.

    SDL_mutexP(mutex);
    run = false;
    SDL_mutexV(mutex);

    // Signal the loader to notice the change.

    SDL_SemPost(sem);
}

//=============================================================================

struct loader_args
{
    uni::buffer_pool  *B;
    uni::needed_queue *N;
    uni::loaded_queue *L;
};

static int loader_func(void *data)
{
    struct loader_args *args = (struct loader_args *) data;

    uni::geomap *M = 0;
    uni::page   *P = 0;

    // Dequeue the next request.

    while (args->N->dequeue(&M, &P))
    {
        // Load and enqueue the page.

        uni::buffer_pool::buff b = args->B->get();

        if (load_png(b.rp, M->name(P)))
            args->L->enqueue(M, P, b);

        // Else mark the page as dead.
    }

    return 0;
}

uni::geocsh::geocsh(int c, int b, int s, int w, int h) :
    c(c), b(b), s(s), S(s + 2), w(w), h(h), n(0), m(w * h), count(0),
    index(new index_line[m]),
    cache(glob->new_image(w * S, h * S, GL_TEXTURE_2D, GL_RGB8, GL_RGB))
{
    cache->bind(GL_TEXTURE0);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    cache->free(GL_TEXTURE0);

    loader_args *args = new loader_args;

    args->B = balloc = new buffer_pool(S, S, c, b);
    args->N = need_Q = new needed_queue();
    args->L = load_Q = new loaded_queue();

    loader = SDL_CreateThread(loader_func, (void *) args);

    debug = ::conf->get_i("debug");
}

uni::geocsh::~geocsh()
{
    need_Q->stop();

    SDL_WaitThread(loader, 0);

    if (need_Q) delete need_Q;
    if (load_Q) delete load_Q;
    if (balloc) delete balloc;

    if (index) delete [] index;

    if (cache) glob->free_image(cache);
}

//-----------------------------------------------------------------------------

void uni::geocsh::init()
{
    // Reset the page index to empty.

    memset(index, 0, m * sizeof (index_line));
    n = 0;
}

void uni::geocsh::seed(const double *vp, double r0, double r1, geomap& map)
{
    // Seed the index with the root page of the given map.

    index[n].M = &map;
    index[n].P =  map.root();
    index[n].k =  map.root()->angle(vp, r0);

    n++;
}

//-----------------------------------------------------------------------------

void uni::geocsh::proc_cache()
{
    // Pop each of the incoming pages from the loader queue.

    geomap *N, *M;
    page   *Q, *P;

    buffer_pool::buff b;

//  while (load_Q->dequeue(&M, &P, &b))
    if (load_Q->dequeue(&M, &P, &b))
    {
        if (cache_map.find(P) == cache_map.end())
        {
            int x = count % w;
            int y = count / w;

            // If the cache is full, delete the LRU page.

            if (count == m)
            {
                Q = cache_lru.front();
                cache_lru.pop_front();

                N = cache_map[Q].M;
                x = cache_map[Q].x;
                y = cache_map[Q].y;

                N->eject_page(Q, x, y);
                cache_map.erase(Q);

                count--;
            }

            // Debug

            static const GLfloat color[][3] = {
                { 1.0f, 0.0f, 0.0f },
                { 1.0f, 0.5f, 0.0f },
                { 1.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 1.0f },
                { 0.0f, 0.0f, 1.0f },
                { 1.0f, 0.0f, 1.0f },
                { 0.0f, 0.0f, 0.0f },
                { 1.0f, 1.0f, 1.0f },
            };

            if (debug)
            {
                glPixelTransferf(GL_RED_SCALE,   color[P->get_d()][0]);
                glPixelTransferf(GL_GREEN_SCALE, color[P->get_d()][1]);
                glPixelTransferf(GL_BLUE_SCALE,  color[P->get_d()][2]);
            }

            // Insert the new page.

            cache_map[P] = cache_line(M, x, y);
            cache_lru.push_back(P);

            M->cache_page(P, x, y);

            cache->blit(b.pp, x * S, y * S, S, S);
            count++;

            if (debug)
            {
                glPixelTransferf(GL_RED_SCALE,   1.0f);
                glPixelTransferf(GL_GREEN_SCALE, 1.0f);
                glPixelTransferf(GL_BLUE_SCALE,  1.0f);
            }
        }

        // Release the image buffer.

        balloc->put(b);
    }
}

void uni::geocsh::proc_index(const double *vp,
                             double r0, double r1, app::frustum_v& frusta)
{
    // While there is still room in the index...

    while (n < m)
    {
        int j = 0;

        // Find the worst page.

        for (int i = 1; i < n; ++i)
            if (index[i].k > index[j].k) j = i;
        
        // If a worst page exists, subdivide it.

        if (index[j].k > 0)
        {
/*
            for (int i = 0; i < n; ++i)
                printf("%4.2f ", index[i].k);
        
            printf("[%d %d %d]\n",
                   index[j].P->get_d(),
                   index[j].P->get_i(),
                   index[j].P->get_j());
*/
            index[j].k = 0;

            for (int i = 0; i < 4 && n < m; ++i)
                if (index[j].P->child(i) && 
                    index[j].P->child(i)->view(frusta, r0, r1))
                {
                    index[n].M = index[j].M;
                    index[n].P = index[j].P->child(i);
                    index[n].k = index[n].P->angle(vp, r0);

                    n++;
                }
        }

        // If no worst page exists, no more subdivision can be done.

        else break;
    }

    // Check if each indexed page is already in the cache.

    for (int i = 0; i < n; ++i)

        if (cache_map.find(index[i].P) == cache_map.end())
        {
            // It is not.  Request it.

            if (!need_Q->find(index[i].P) &&
                !load_Q->find(index[i].P))
                need_Q->enqueue(index[i].M, index[i].P);
        }
        else
        {
            // It is. Bump it to the end of the LRU queue.

            cache_lru.remove   (index[i].P);
            cache_lru.push_back(index[i].P);
        }
}

void uni::geocsh::proc(const double *vp,
                       double r0, double r1, app::frustum_v& frusta)
{
    proc_index(vp, r0, r1, frusta);
    proc_cache();
}

//-----------------------------------------------------------------------------

void uni::geocsh::bind(GLenum unit) const
{
    cache->bind(unit);

    ogl::program::current->uniform("pool_size", w * S, h * S);
}

void uni::geocsh::free(GLenum unit) const
{
    cache->free(unit);
}

void uni::geocsh::draw() const
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        cache->bind(GL_TEXTURE0);
        {
            glMatrixMode(GL_PROJECTION);
            {
                glPushMatrix();
                glLoadIdentity();
            }
            glMatrixMode(GL_TEXTURE);
            {
                glPushMatrix();
                glLoadIdentity();
            } 
            glMatrixMode(GL_MODELVIEW);
            {
                glPushMatrix();
                glLoadIdentity();
            }

            glBegin(GL_QUADS);
            {
                glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex2f(+0.0f, -1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(+0.0f, +0.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, +0.0f);
            }
            glEnd();

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_TEXTURE);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }
        }
        cache->free(GL_TEXTURE0);
    }
    glPopAttrib();
}

void uni::geocsh::wire(double r0, double r1) const
{
    for (int i = 0; i < n; ++i)
        if (index[i].P->get_d() == 6)
            index[i].P->draw(r0, r1);
}

//-----------------------------------------------------------------------------
