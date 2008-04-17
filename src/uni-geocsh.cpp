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

#include "uni-geocsh.hpp"
#include "app-conf.hpp"
#include "app-host.hpp"

//=============================================================================
// Data loader threads query the cache object for requests, load the requested
// data, and submit the loaded buffer back to the cache object.

static int loader_func(void *data)
{
    uni::geocsh *C = (uni::geocsh *) data;
    uni::geomap *M = 0;
    uni::page   *P = 0;
    uni::buffer *B = 0;

    while (C->get_needed(&M, &P, &B))
           C->put_loaded( M,  P,  B->load(M->name(P)));

    return 0;
}

//=============================================================================
// Geometry data buffer

uni::buffer::buffer(int w, int h, int c, int b) :
    w(png_uint_32(w)),
    h(png_uint_32(h)),
    c(png_byte(c)),
    b(png_byte(b)),
    ret(false)
{
    // Allocate a pixel buffer and an array of row pointers.

    dat = new GLubyte[w * h * c * b];
    row = new png_bytep[h];

    // Initialize the row pointers to index the pixel buffer upside-down.

    if (dat && row)
        for (int i = 0; i < h; ++i)
            row[h - i - 1] = dat + i * w * c * b;
}

uni::buffer::~buffer()
{
    if (row) delete [] row;
    if (dat) delete [] dat;
}

uni::buffer *uni::buffer::load(std::string name)
{
    png_structp rp = 0;
    png_infop   ip = 0;
    FILE       *fp = 0;

    ret = false;

//  std::cout << name << std::endl;

    // Initialize all PNG import data structures.

    if (dat && row && (fp = fopen(name.c_str(), "rb")))
    {
        if ((rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        {
            if ((ip = png_create_info_struct(rp)))
            {
                if (setjmp(png_jmpbuf(rp)) == 0)
                {
                    // Read the PNG.

                    png_init_io  (rp, fp);
                    png_read_info(rp, ip);

                    if (png_get_image_width (rp, ip) == w &&
                        png_get_image_height(rp, ip) == h &&
                        png_get_channels    (rp, ip) == c &&
                        png_get_bit_depth   (rp, ip) == b * 8)
                    {
                        png_read_image(rp, row);
                        ret = true;
                    }
                }
                png_destroy_read_struct(&rp, &ip, 0);
            }
        }
        fclose(fp);
    }

    return this;
}

//=============================================================================

uni::geocsh::geocsh(int c, int b, int s, int w, int h) :
    c(c), b(b), s(s), S(s + 2), w(w), h(h), n(0), m(w * h), count(0), run(true),
    cache(glob->new_image(w * S, h * S, GL_TEXTURE_2D, GL_RGB8, GL_RGB))
{
    cache->bind(GL_TEXTURE0);
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    cache->free(GL_TEXTURE0);

    debug = ::conf->get_i("debug");

    // Initialize the IPC.

    need_cond   = SDL_CreateCond();
    need_mutex  = SDL_CreateMutex();
    load_mutex  = SDL_CreateMutex();
    buff_mutex  = SDL_CreateMutex();

    // Start the data loader threads.

    int threads = std::max(1, ::conf->get_i("loader_threads"));

    for (int i = 0; i < threads; ++i)
        load_thread.push_back(SDL_CreateThread(loader_func, (void *) this));
}

uni::geocsh::~geocsh()
{
    run = false;

    // Signal the loader threads to exit.  Join them.

    SDL_CondBroadcast(need_cond);

    std::vector<SDL_Thread *>::iterator i;

    for (i = load_thread.begin(); i != load_thread.end(); ++i)
        SDL_WaitThread(*i, 0);

    // Release all our IPC.

    SDL_DestroyMutex(buff_mutex);
    SDL_DestroyMutex(load_mutex);
    SDL_DestroyMutex(need_mutex);
    SDL_DestroyCond (need_cond);

    // TODO: free the buffer list

    if (cache) glob->free_image(cache);
}

//-----------------------------------------------------------------------------

void uni::geocsh::init()
{
    // Reset the needed pages.

    SDL_mutexP(need_mutex);
    needs.clear();
    SDL_mutexV(need_mutex);

    n = 0;
}

void uni::geocsh::seed(const double *vp, double r0, double r1, geomap& map)
{
    // Seed the needed pages with the root page of the given map.

    geomap *M = &map;
    page   *P =  map.root();

    SDL_mutexP(need_mutex);
    needs.insert(need_map::value_type(P->angle(vp, r0), need(M, P)));
    SDL_mutexV(need_mutex);

    n++;
}

//-----------------------------------------------------------------------------

void uni::geocsh::proc_loads()
{
    // Pop each of the loaded pages from the queue.

    while (!loads.empty())
    {
        geomap *M = loads.front().M;
        page   *P = loads.front().P;
        buffer *B = loads.front().B;

        loads.pop_front();

        // Debug mode scales the page color to make depth apparent.

        if (debug)
        {
            static const GLfloat color[][3] = {
                { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.5f, 0.0f },
                { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
                { 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
                { 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },
            };

            glPixelTransferf(GL_RED_SCALE,   color[P->get_d()][0]);
            glPixelTransferf(GL_GREEN_SCALE, color[P->get_d()][1]);
            glPixelTransferf(GL_BLUE_SCALE,  color[P->get_d()][2]);
        }

        // If the page was correctly loaded...

        if (B->stat())
        {
            // If this page is not already cached (highly unlikely)...

            if (cache_idx.find(P) == cache_idx.end())
            {
                int x = count % w;
                int y = count / w;

                // If the cache is full, delete the LRU page.

                if (count == m)
                {
                    page   *Q = cache_lru.front();
                    geomap *N = cache_idx[Q].M;

                    x = cache_idx[Q].x;
                    y = cache_idx[Q].y;

                    N->eject_page(Q, x, y);

                    cache_idx.erase(Q);
                    cache_lru.pop_front();

                    count--;
                }

                // Insert the new page.

                cache_idx.insert(line_map::value_type(P, line(M, x, y)));
                cache_lru.push_back(P);

                M->cache_page(P, x, y);

                cache->blit(B->data(), x * S, y * S, S, S);
                count++;
            }
        }
        // TODO: mark the page as dead.

        // Release the image buffer.

        SDL_mutexP(buff_mutex);
        buffs.push_front(B);
        SDL_mutexV(buff_mutex);
    }

    // Revert the debug mode scale.

    if (debug)
    {
        glPixelTransferf(GL_RED_SCALE,   1.0f);
        glPixelTransferf(GL_GREEN_SCALE, 1.0f);
        glPixelTransferf(GL_BLUE_SCALE,  1.0f);
    }
}

void uni::geocsh::proc_needs(const double *vp,
                             double r0, double r1, app::frustum_v& frusta)
{
    // While the needed page count is less than the limit.

    while (n < m)
    {
        // If a worst page exists...

        need_map::iterator i = needs.begin();

        if (i->first > 0)
        {
            need L = i->second;

            // Eliminate it from further consideration.

            needs.erase (i);
            needs.insert(need_map::value_type(0, L));

            // Subdivide it.

            for (int i = 0; i < 4 && n < m; ++i)

                if (L.P->child(i) && L.P->child(i)->view(frusta, r0, r1))
                {
                    need C(L.M, L.P->child(i));

                    double k = C.P->angle(vp, r0);

                    needs.insert(need_map::value_type(k, C));
                    n++;
                }
        }

        // If no worst page exists, no more subdivision can be done.

        else break;
    }

    // Check if any needed page is already in the cache.

    for (need_map::iterator i = needs.begin(); i != needs.end();)
    {
        page *P = i->second.P;

        if (cache_idx.find(P) != cache_idx.end())
        {
            // If found, bump it to the end of the LRU queue.

            cache_lru.remove   (P);
            cache_lru.push_back(P);

            // And remove it from the needs map.

            needs.erase(i++);
            n--;
        }
        else ++i;
    }
}

void uni::geocsh::proc(const double *vp,
                       double r0, double r1, app::frustum_v& frusta)
{
    // Process all loaded pages.

    SDL_mutexP(load_mutex);
    proc_loads();
    SDL_mutexV(load_mutex);

    // Process all needed pages.

    SDL_mutexP(need_mutex);
    proc_needs(vp, r0, r1, frusta);
    SDL_mutexV(need_mutex);

    // If there are outstanding pages, signal the page loader threads.

    if (n) SDL_CondBroadcast(need_cond);
}

//-----------------------------------------------------------------------------

bool uni::geocsh::get_needed(geomap **M, page **P, buffer **B)
{
    // Wait for the needed page map to be non-empty and remove the first.

    SDL_mutexP(need_mutex);
    {
        SDL_CondWait(need_cond, need_mutex);

        if (!needs.empty())
        {
            *M = needs.begin()->second.M;
            *P = needs.begin()->second.P;

            needs.erase(needs.begin());
            n--;
        }
    }
    SDL_mutexV(need_mutex);

    // Acquire a buffer to receive the loaded page.

    SDL_mutexP(buff_mutex);
    {
        if (buffs.empty())
            *B = new buffer(S, S, c, b);
        else
        {
            *B = buffs.front();
             buffs.pop_front();
        }
    }
    SDL_mutexV(buff_mutex);

    return run;
}

void uni::geocsh::put_loaded(geomap *M, page *P, buffer *B)
{
    // Add the loaded page to the incoming queue.

    SDL_mutexP(load_mutex);
    loads.push_back(load(M, P, B));
    SDL_mutexV(load_mutex);
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
    GLfloat a = (GLfloat(::host->get_window_h() * w) /
                 GLfloat(::host->get_window_w() * h));

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
                glTexCoord2f(1.0f, 0.0f); glVertex2f(-1.0f + a,     -1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex2f(-1.0f + a,     +0.0f);
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
/*
    for (int i = 0; i < n; ++i)
        if (index[i].P->get_d() == 6)
            index[i].P->draw(r0, r1);
*/
}

//-----------------------------------------------------------------------------
