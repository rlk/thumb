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

//=============================================================================

static unsigned char *load_png(std::string name)
{
    unsigned char *p = 0;
    png_structp   rp = 0;
    png_infop     ip = 0;
    png_bytep    *bp = 0;
    FILE         *fp = 0;

    std::cout << name << std::endl;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(name.c_str(), "rb")))
        return 0;

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return 0;

    if (!(ip = png_create_info_struct(rp)))
        return 0;

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        // Read the PNG header.

        png_init_io (rp, fp);
        png_read_png(rp, ip, PNG_TRANSFORM_PACKING |
                             PNG_TRANSFORM_SWAP_ENDIAN, 0);
        
        // Extract image properties.

        GLsizei w = GLsizei(png_get_image_width (rp, ip));
        GLsizei h = GLsizei(png_get_image_height(rp, ip));
        GLsizei b = GLsizei(png_get_bit_depth   (rp, ip)) / 8;
        GLsizei c = 1;

        switch (png_get_color_type(rp, ip))
        {
        case PNG_COLOR_TYPE_GRAY:       c = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: c = 2; break;
        case PNG_COLOR_TYPE_RGB:        c = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  c = 4; break;
        default: throw std::runtime_error("Unsupported PNG color type");
        }

        // Allocate a buffer and copy the pixel data to it.

        if ((p = (unsigned char *) malloc(w * h * c * b)))
        {
            if ((bp = png_get_rows(rp, ip)))
                for (int i = 0, r = h - 1; i < h; ++i, --r)
                    memcpy(p + i * w * c * b, bp[r], w * c * b);
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, 0);
    fclose(fp);

    return p;
}

//=============================================================================

struct uni::loaded_queue::load
{
    geomap        *M;
    page          *P;
    unsigned char *D;

    load(geomap *M, page *P, unsigned char *D) : M(M), P(P), D(D) { }
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

void uni::loaded_queue::enqueue(geomap *M, page *P, unsigned char *D)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    Q.push(load(M, P, D));
    SDL_mutexV(mutex);
}

bool uni::loaded_queue::dequeue(geomap **M, page **P, unsigned char **D)
{
    bool b = false;

    // If the queue is not empty, dequeue.

    SDL_mutexP(mutex);

    if (!Q.empty())
    {
        load L = Q.front(); Q.pop();

        *M = L.M;
        *P = L.P;
        *D = L.D;
         b = true;
    }

    SDL_mutexV(mutex);

    return b;
}

//=============================================================================

struct uni::needed_queue::need
{
    loaded_queue *L;
    geomap       *M;
    page         *P;

    need(loaded_queue *L, geomap *M, page *P) : L(L), M(M), P(P) { }
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

void uni::needed_queue::enqueue(loaded_queue *L, geomap *M, page *P)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    {
        bool b = true;

        for (std::list<need>::iterator i = Q.begin(); i != Q.end(); ++i)
            if (i->P == P) b = false;

        if (b) Q.push_back(need(L, M, P));
    }
    SDL_mutexV(mutex);

    // Signal the consumer.

    SDL_SemPost(sem);
}

bool uni::needed_queue::dequeue(loaded_queue **L, geomap **M, page **P)
{
    bool b;

    // Wait for the signal to proceed.

    SDL_SemWait(sem);

    // Dequeue the request.

    SDL_mutexP(mutex);
    if ((b = run))
    {
        *L = Q.front().L;
        *M = Q.front().M;
        *P = Q.front().P;
        Q.pop_front();
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

static int loader_func(void *data)
{
    uni::needed_queue *N = (uni::needed_queue *) data;
    uni::loaded_queue *L = 0;
    uni::geomap       *M = 0;
    uni::page         *P = 0;

    // Dequeue the next request.

    while (N->dequeue(&L, &M, &P))
    {
        // Load and enqueue the page.

        if (L && M && P)
            L->enqueue(M, P, load_png(M->name(P)));
    }

    return 0;
}

//=============================================================================

uni::geocsh::geocsh(int c, int b, int s, int w, int h) :
    c(c), b(b), s(s), w(w), h(h), n(0), m(w * h), index(new index_line[m]),
    cache(glob->new_image(w * s, h * s, GL_TEXTURE_2D, GL_RGB))
{
    need_Q = new needed_queue();
    load_Q = new loaded_queue();

    loader = SDL_CreateThread(loader_func, (void *) need_Q);
}

uni::geocsh::~geocsh()
{
    need_Q->stop();

    SDL_WaitThread(loader, 0);

    if (need_Q) delete need_Q;
    if (load_Q) delete load_Q;

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

void uni::geocsh::proc(const double *vp,
                       double r0, double r1, app::frustum_v& frusta)
{
    // Pop each of the incoming pages from the loader queue.

    geomap        *M;
    page          *P;
    unsigned char *D;

    while (load_Q->dequeue(&M, &P, &D))
    {
        if (D) ::free(D);
    }

    // While there is still room in the cache...

    while (n < m)
    {
        int j = 0;

        // Find the worst page.

        for (int i = 1; i < n; ++i)
            if (index[i].k > index[j].k) j = i;
        
        // If a worst page exists, subdivide it.

        if (index[j].k > 0)
        {
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

    // Check if each desired page is already in the cache.

    for (int i = 0; i < n; ++i)

        if (cache_map.find(index[i].P) == cache_map.end())
        {
            // It is not.  Request it.

            need_Q->enqueue(load_Q, index[i].M, index[i].P);
        }
        else
        {
            // It is. Bump it to the end of the LRU queue.

            cache_lru.remove   (index[i].P);
            cache_lru.push_back(index[i].P);
        }
}

//-----------------------------------------------------------------------------
