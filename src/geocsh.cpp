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

        // TODO: consider inverting the pngs to make this unnecessary.
        // TODO: consider byte order interleaving for optimal upload.

        if ((p = (unsigned char *) malloc(w * h * c * b)))
        {
            if ((bp = png_get_rows(rp, ip)))
                for (int i = 0, r = h - 1; i < h; ++i, --r)
                    memcpy(p + i * w * c * b, bp[r], w * c * b);
        }

//      printf("%s %d %d %d %d\n", name.c_str(), w, h, b, c);
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

void uni::loaded_queue::enqueue(geomap *M, page *P, unsigned char *D)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    Q.push_back(load(M, P, D));
    SDL_mutexV(mutex);
}

bool uni::loaded_queue::dequeue(geomap **M, page **P, unsigned char **D)
{
    bool b = false;

    // If the queue is not empty, dequeue.

    SDL_mutexP(mutex);
    {
        if (!Q.empty())
        {
            *M = Q.front().M;
            *P = Q.front().P;
            *D = Q.front().D;

            Q.pop_front();

            b = true;
        }
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

void uni::needed_queue::enqueue(loaded_queue *L, geomap *M, page *P)
{
    bool b = true;

    // Enqueue the request.

    SDL_mutexP(mutex);
    {
        for (std::list<need>::iterator i = Q.begin(); i != Q.end(); ++i)
            if (i->P == P) b = false;

        if (b) Q.push_back(need(L, M, P));
    }
    SDL_mutexV(mutex);

    // Signal the consumer.

    if (b) SDL_SemPost(sem);
}

bool uni::needed_queue::dequeue(loaded_queue **L, geomap **M, page **P)
{
    bool b;

    // Wait for the signal to proceed.

    SDL_SemWait(sem);

    // Dequeue the request.

    SDL_mutexP(mutex);
    {
        if ((b = run))
        {
            *L = Q.front().L;
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
    c(c), b(b), s(s), S(s + 2), w(w), h(h), n(0), m(w * h), count(0),
    index(new index_line[m]),
    cache(glob->new_image(w * S, h * S, GL_TEXTURE_2D, GL_RGB8, GL_RGB))
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

//-----------------------------------------------------------------------------

void uni::geocsh::proc_cache()
{
    // Pop each of the incoming pages from the loader queue.

    geomap    *N, *M;
    page      *Q, *P;
    unsigned char *D;

    while (load_Q->dequeue(&M, &P, &D) && D)
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

            // Insert the new page.

            cache_map[P] = cache_line(M, x, y);
            cache_lru.push_back(P);

            M->cache_page(P, x, y);

            cache->blit(D, x * S, y * S, S, S);
            count++;
        }

        // Release the image buffer.

        ::free(D);
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
                need_Q->enqueue(load_Q, index[i].M, index[i].P);
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

//-----------------------------------------------------------------------------
