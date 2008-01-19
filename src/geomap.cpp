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
#include <iomanip>
#include <sstream>
#include <string>
#include <png.h>

#include "geomap.hpp"
#include "default.hpp"
#include "program.hpp"
#include "matrix.hpp"
#include "util.hpp"

//=============================================================================

uni::texture_pool::texture_pool(int n, int s, int c, int b) :
    n(n), s(s), c(c), b(b)
{
    static const GLenum type_tag[2] = {
        GL_UNSIGNED_BYTE,
        GL_UNSIGNED_SHORT
    };
    static const GLenum form_tag[2][4] = {
        { GL_LUMINANCE,   GL_LUMINANCE_ALPHA,     GL_RGB,   GL_RGBA   },
        { GL_LUMINANCE16, GL_LUMINANCE16_ALPHA16, GL_RGB16, GL_RGBA16 },
    };

    const GLenum fi   = form_tag[b - 1][c - 1];
    const GLenum fe   = form_tag[0    ][c - 1];
    const GLenum type = type_tag[b - 1];
    
    pool = new GLuint[n];
    stat = new bool  [n];

    glGenTextures(n, pool);

    for (int i = 0; i < n; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, pool[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, fi, s + 2, s + 2, 1, fe, type, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP);

        stat[i] = false;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

uni::texture_pool::~texture_pool()
{
    glDeleteTextures(n, pool);
    delete [] stat;
    delete [] pool;
}

GLuint uni::texture_pool::get()
{
    for (int i = 0; i < n; ++i)
        if (stat[i] == false)
        {
            stat[i] = true;
            return pool[i];
        }

    return 0;
}

void uni::texture_pool::put(GLuint o)
{
    for (int i = 0; i < n; ++i)
        if (pool[i] == o)
        {
            stat[i] = false;
            break;
        }
}

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

//-----------------------------------------------------------------------------

static int loader_func(void *data)
{
    uni::needed_queue *N = (uni::needed_queue *) data;

    std::string     name;
    uni::loaded_queue *L = 0;
    uni::page         *P = 0;

    // Dequeue the next request.

    while (N->dequeue(name, &L, &P))
    {
        // Load and enqueue the page.

        if (L && P) L->enqueue(P, load_png(name));
    }

    return 0;
}

//=============================================================================

struct uni::loaded_queue::load
{
    page          *P;
    unsigned char *D;

    load(page *P, unsigned char *D) : P(P), D(D) { }
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

void uni::loaded_queue::enqueue(page *P, unsigned char *D)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    Q.push(load(P, D));
    SDL_mutexV(mutex);
}

bool uni::loaded_queue::dequeue(page **P, unsigned char **D)
{
    bool b = false;

    // If the queue is not empty, dequeue.

    SDL_mutexP(mutex);

    if (!Q.empty())
    {
        load L = Q.front(); Q.pop();

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
    std::string name;
    loaded_queue *L;
    page         *P;

    need(std::string& name, loaded_queue *L, page *P) :
        name(name), L(L), P(P) { }
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

void uni::needed_queue::enqueue(std::string& name, loaded_queue *L, page *P)
{
    // Enqueue the request.

    SDL_mutexP(mutex);
    {
        Q.push(need(name, L, P));
    }
    SDL_mutexV(mutex);

    // Signal the consumer.

    SDL_SemPost(sem);
}

bool uni::needed_queue::dequeue(std::string& name, loaded_queue **L, page **P)
{
    bool b;

    // Wait for the signal to proceed.

    SDL_SemWait(sem);

    // Dequeue the request.

    SDL_mutexP(mutex);
    if ((b = run))
    {
        name = Q.front().name;
        *L   = Q.front().L;
        *P   = Q.front().P;
        Q.pop();
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

int uni::page::count;

uni::page::page(page *U,
                int w, int h, int s,
                int x, int y, int d,
                double r0, double r1,
                double _L, double _R, double _B, double _T) :
    state(dead_state), U(U), d(d),
    object(0), hint(0),
    is_visible(false), is_valued(false)
{
    int S = s << d;

    // Compute the mipmap file indices.

    i = y / S;
    j = x / S;

    // Compute the texture boundries.

    L = (_L + (_R - _L) * (x    ) / w);
    R = (_L + (_R - _L) * (x + S) / w);
    T = (_T + (_B - _T) * (y    ) / h);
    B = (_T + (_B - _T) * (y + S) / h);

    // Create subpages as necessary.

    P[0] = 0;
    P[1] = 0;
    P[2] = 0;
    P[3] = 0;

    if (d > 0)
    {
        const int X = (x + S / 2);
        const int Y = (y + S / 2);

        /* Has children? */ P[0] = new page(this, w, h, s, x, y, d-1,
                                            r0, r1, _L, _R, _B, _T);
        if (         Y < h) P[1] = new page(this, w, h, s, x, Y, d-1,
                                            r0, r1, _L, _R, _B, _T);
        if (X < w         ) P[2] = new page(this, w, h, s, X, y, d-1,
                                            r0, r1, _L, _R, _B, _T);
        if (X < w && Y < h) P[3] = new page(this, w, h, s, X, Y, d-1,
                                            r0, r1, _L, _R, _B, _T);

        // Accumulate the AABBs and areas of the children. */

        b[0] =  std::numeric_limits<double>::max();
        b[1] =  std::numeric_limits<double>::max();
        b[2] =  std::numeric_limits<double>::max();
        b[3] = -std::numeric_limits<double>::max();
        b[4] = -std::numeric_limits<double>::max();
        b[5] = -std::numeric_limits<double>::max();

        area = 0.0;

        for (int k = 0; k < 4; ++k)
            if (P[k])
            {
                b[0] = std::min(b[0], P[k]->b[0]);
                b[1] = std::min(b[1], P[k]->b[1]);
                b[2] = std::min(b[2], P[k]->b[2]);
                b[3] = std::max(b[3], P[k]->b[3]);
                b[4] = std::max(b[4], P[k]->b[4]);
                b[5] = std::max(b[5], P[k]->b[5]);

                area += P[k]->area;
            }
    }
    else
    {
        // Compute the AABB of this page.

        double v[8][3];

        // TODO: watch out for R > pi and B < -pi/2
        // TODO: r2 instead of r1 here?

        sphere_to_vector(v[0], L, B, r0);
        sphere_to_vector(v[1], R, B, r0);
        sphere_to_vector(v[2], L, T, r0);
        sphere_to_vector(v[3], R, T, r0);
        sphere_to_vector(v[4], L, B, r1);
        sphere_to_vector(v[5], R, B, r1);
        sphere_to_vector(v[6], L, T, r1);
        sphere_to_vector(v[7], R, T, r1);

        b[0] = min8(v[0][0], v[1][0], v[2][0], v[3][0],
                    v[4][0], v[5][0], v[6][0], v[7][0]);
        b[1] = min8(v[0][1], v[1][1], v[2][1], v[3][1],
                    v[4][1], v[5][1], v[6][1], v[7][1]);
        b[2] = min8(v[0][2], v[1][2], v[2][2], v[3][2],
                    v[4][2], v[5][2], v[6][2], v[7][2]);
        b[3] = max8(v[0][0], v[1][0], v[2][0], v[3][0],
                    v[4][0], v[5][0], v[6][0], v[7][0]);
        b[4] = max8(v[0][1], v[1][1], v[2][1], v[3][1],
                    v[4][1], v[5][1], v[6][1], v[7][1]);
        b[5] = max8(v[0][2], v[1][2], v[2][2], v[3][2],
                    v[4][2], v[5][2], v[6][2], v[7][2]);

        // Compute the (trapezoidal approximate) area of this page.

        double lt = (r0 + r1) * sin((R - L) * cos(T) * 0.5);
        double lb = (r0 + r1) * sin((R - L) * cos(B) * 0.5);
        double lh = (r0 + r1) * sin((T - B)          * 0.5);

        area = (lt + lb) * lh * 0.5;
    }

    // Compute the center of this page.

    c[0] = (b[0] + b[3]) / 2.0;
    c[1] = (b[1] + b[4]) / 2.0;
    c[2] = (b[2] + b[5]) / 2.0;
/*
    object = debug_text[d];
    state  = live_state;
*/
}

uni::page::~page()
{
    if (P[3]) delete P[3];
    if (P[2]) delete P[2];
    if (P[1]) delete P[1];
    if (P[0]) delete P[0];

    remove();
}

void uni::page::assign(GLuint o)
{
    object = o;
    state  = live_state;
}

GLuint uni::page::remove()
{
    state  = dead_state;
    return object;
}

void uni::page::ignore()
{
    state  = skip_state;
}

//-----------------------------------------------------------------------------

bool uni::page::value(const double *p)
{
    double q[3];

    double bias = 0.5;

    q[0] = c[0] - p[0];
    q[1] = c[1] - p[1];
    q[2] = c[2] - p[2];

    double k = bias * area / DOT3(q, q);

    return (k > 1);
}

bool uni::page::test(const double *P)
{
    // Determine which AABB corner is front-most and test it.

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return (P[0] * b[3] + P[1] * b[4] + P[2] * b[5] + P[3] < 0);
            else
                return (P[0] * b[3] + P[1] * b[4] + P[2] * b[2] + P[3] < 0);
        else
            if (P[2] > 0)
                return (P[0] * b[3] + P[1] * b[1] + P[2] * b[5] + P[3] < 0);
            else
                return (P[0] * b[3] + P[1] * b[1] + P[2] * b[2] + P[3] < 0);
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return (P[0] * b[0] + P[1] * b[4] + P[2] * b[5] + P[3] < 0);
            else
                return (P[0] * b[0] + P[1] * b[4] + P[2] * b[2] + P[3] < 0);
        else
            if (P[2] > 0)
                return (P[0] * b[0] + P[1] * b[1] + P[2] * b[5] + P[3] < 0);
            else
                return (P[0] * b[0] + P[1] * b[1] + P[2] * b[2] + P[3] < 0);
}

bool uni::page::visible(const double *V, const double *p)
{
    // If we're inside the bound, the page is visible.

    if (b[0] < p[0] && p[0] < b[3] &&
        b[1] < p[1] && p[1] < b[4] &&
        b[2] < p[2] && p[2] < b[5]) return true;

    // Use the hint to check the likely cull plane.

    if (test(V + hint)) return false;

    // The hint was no good.  Test the other planes and update the hint. 

    if (hint !=  0 && test(V +  0)) { hint =  0; return false; }
    if (hint !=  4 && test(V +  4)) { hint =  4; return false; }
    if (hint !=  8 && test(V +  8)) { hint =  8; return false; }
    if (hint != 12 && test(V + 12)) { hint = 12; return false; }
    if (hint != 16 && test(V + 16)) { hint = 16; return false; }
    if (hint != 20 && test(V + 20)) { hint = 20; return false; }

    // Nothing clipped.  Return visible.

    return true;
}

bool uni::page::will_draw(int d1) const
{
    return (d <= d1 && is_valued && state == live_state);
}

//-----------------------------------------------------------------------------

void uni::page::volume() const
{
    glBegin(GL_QUADS);
    {
        // -X
        glVertex3d(b[0], b[1], b[2]);
        glVertex3d(b[0], b[1], b[5]);
        glVertex3d(b[0], b[4], b[5]);
        glVertex3d(b[0], b[4], b[2]);

        // +X
        glVertex3d(b[3], b[1], b[5]);
        glVertex3d(b[3], b[1], b[2]);
        glVertex3d(b[3], b[4], b[2]);
        glVertex3d(b[3], b[4], b[5]);

        // -Y
        glVertex3d(b[0], b[1], b[2]);
        glVertex3d(b[3], b[1], b[2]);
        glVertex3d(b[3], b[1], b[5]);
        glVertex3d(b[0], b[1], b[5]);

        // +Y
        glVertex3d(b[0], b[4], b[5]);
        glVertex3d(b[3], b[4], b[5]);
        glVertex3d(b[3], b[4], b[2]);
        glVertex3d(b[0], b[4], b[2]);

        // -Z
        glVertex3d(b[3], b[1], b[2]);
        glVertex3d(b[0], b[1], b[2]);
        glVertex3d(b[0], b[4], b[2]);
        glVertex3d(b[3], b[4], b[2]);

        // +Z
        glVertex3d(b[0], b[1], b[5]);
        glVertex3d(b[3], b[1], b[5]);
        glVertex3d(b[3], b[4], b[5]);
        glVertex3d(b[0], b[4], b[5]);
    }
    glEnd();
}

void uni::page::prep(const double *r, int d0, int d1)
{
    if (d >= d0)
    {
        if (state != skip_state)
        {
            // Compute the range where-in this page is necessary.

            double dt = (R - L) * 0.5;
            double dp = (T - B) * 0.5;

            double t0 = L - dt;
            double t1 = R + dt;
            double p0 = B - dp;
            double p1 = T + dp;

            // Don't test past the poles.

            if (p0 < -PI / 2) p0 = -PI / 2;
            if (p1 >  PI / 2) p1 =  PI / 2;

            // Exercise caution at the international date line.

            double ta = r[0];
            double tb = r[0] + 2 * PI;
            double tc = r[0] - 2 * PI;

            is_valued = (((t0 < ta && ta < t1) ||
                          (t0 < tb && tb < t1) ||
                          (t0 < tc && tc < t1)) && (p0 < r[1] && r[1] < p1));
        }

        if (P[0]) P[0]->prep(r, d0, d1);
        if (P[1]) P[1]->prep(r, d0, d1);
        if (P[2]) P[2]->prep(r, d0, d1);
        if (P[3]) P[3]->prep(r, d0, d1);
    }
}

void uni::page::draw(geomap& M, int w, int h, int d0, int d1)
{
    if (d >= d0)
    {
/*
        if (P[0]) P[0]->draw(M, w, h, d0, d1);
        if (P[1]) P[1]->draw(M, w, h, d0, d1);
        if (P[2]) P[2]->draw(M, w, h, d0, d1);
        if (P[3]) P[3]->draw(M, w, h, d0, d1);
*/
        int c = 0;

        if (P[0] && P[0]->will_draw(d1)) c++;
        if (P[1] && P[1]->will_draw(d1)) c++;
        if (P[2] && P[2]->will_draw(d1)) c++;
        if (P[3] && P[3]->will_draw(d1)) c++;

        if (c < 4 && is_valued && d <= d1)
        {
            // If this page should draw but has no texture, request it.

            if (state == dead_state)
            {
                if (M.needed(this, d, i, j))
                    state = wait_state;
            }

            if (state == live_state)
            {
                // Set up this page's texture transform.

                double kt =  1 / (R - L);
                double kp =  1 / (T - B);
                double dt = -kt * L;
                double dp = -kp * B;

                glActiveTextureARB(GL_TEXTURE1);
                {
                    glBindTexture(GL_TEXTURE_2D, object);
                }
                glActiveTextureARB(GL_TEXTURE0);

                // TODO: store this uniform location

                ogl::program::current->uniform("d", dt, dp);
                ogl::program::current->uniform("k", kt, kp);

                // Draw this page's bounding volume.

                if (w > 0 && h > 0)
                    glRecti(0, 0, w, h);
                else
                    volume();

                count++;

                M.used(this);
            }
        }

        if (P[0]) P[0]->draw(M, w, h, d0, d1);
        if (P[1]) P[1]->draw(M, w, h, d0, d1);
        if (P[2]) P[2]->draw(M, w, h, d0, d1);
        if (P[3]) P[3]->draw(M, w, h, d0, d1);
    }
}

void uni::page::wire()
{
    if (is_valued)
    {
        volume();

        if (P[0]) P[0]->wire();
        if (P[1]) P[1]->wire();
        if (P[2]) P[2]->wire();
        if (P[3]) P[3]->wire();
    }
}

//=============================================================================

SDL_Thread        *uni::geomap::loader = 0;
uni::needed_queue *uni::geomap::need_Q = 0;

void uni::geomap::init()
{
    need_Q = new needed_queue();
    loader = SDL_CreateThread(loader_func, (void *) need_Q);
}

void uni::geomap::fini()
{
    need_Q->stop();
    SDL_WaitThread(loader, 0);

    delete need_Q;
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name,
                    int w, int h, int c, int b, int s,
                    double k, double r0, double r1,
                    double L, double R, double B, double T) :
    name(name), debug(false), c(c), b(b), s(s), d(0), k(k), r0(r0), r1(r1)
{
    if (loader == 0) init();

    // Compute the depth of the mipmap pyramid.

    int S = s;

    while (S < w || S < h)
    {
        S *= 2;
        d += 1;
    }

    // Generate the mipmap pyramid catalog.

    P = new page(0, w, h, s, 0, 0, d, r0, r1, L, R, B, T);

    // Initialize the load queue.

    load_Q = new loaded_queue;
    text_P = new texture_pool(DEFAULT_TEXTURE_POOL, s, c, b);
}

uni::geomap::~geomap()
{
    if (text_P) delete(text_P);
    if (load_Q) delete(load_Q);
    if (P)      delete P;
}

//-----------------------------------------------------------------------------

bool uni::geomap::needed(page *P, int d, int i, int j)
{
    // Construct the file name.

    std::ostringstream file;

    file << name
         << "-" << std::hex << std::setfill('0') << std::setw(2) << d
         << "-" << std::hex << std::setfill('0') << std::setw(2) << i
         << "-" << std::hex << std::setfill('0') << std::setw(2) << j << ".png";

    // Queue the request.

    if (need_Q)
    {
        std::string name = file.str();

        need_Q->enqueue(name, load_Q, P);
        return true;
    }
    return false;
}

bool uni::geomap::loaded()
{
    static const GLubyte color[][4] = {
        { 0xFF, 0xFF, 0xFF, 0xFF },
        { 0xFF, 0xFF, 0x00, 0xFF },
        { 0x00, 0xFF, 0xFF, 0xFF },
        { 0xFF, 0x00, 0xFF, 0xFF },
        { 0xFF, 0x7F, 0x00, 0xFF },
        { 0x00, 0x7F, 0xFF, 0xFF },
        { 0x7F, 0x00, 0xFF, 0xFF },
        { 0xFF, 0x00, 0x7F, 0xFF },
        { 0x7F, 0xFF, 0x00, 0xFF },
        { 0x00, 0xFF, 0x7F, 0xFF },
        { 0xFF, 0x00, 0x00, 0xFF },
        { 0x00, 0xFF, 0x00, 0xFF },
        { 0x00, 0x00, 0xFF, 0xFF },
    };
    static const GLenum type_tag[2] = {
        GL_UNSIGNED_BYTE,
        GL_UNSIGNED_SHORT
    };
    static const GLenum form_tag[2][4] = {
        { GL_LUMINANCE,   GL_LUMINANCE_ALPHA,     GL_RGB,   GL_RGBA   },
        { GL_LUMINANCE16, GL_LUMINANCE16_ALPHA16, GL_RGB16, GL_RGBA16 },
    };
/*
    static const GLenum form_tag[4] = {
        GL_LUMINANCE,
        GL_LUMINANCE_ALPHA,
        GL_RGB,
        GL_RGBA
    };
*/
    page          *P;
    unsigned char *D;

    // Check to see if any recently loaded pages are ready.

    if (load_Q && load_Q->dequeue(&P, &D))
    {
        // If we've got data, download the page to the GL.
        
        if (D)
        {
            const GLenum fi   = form_tag[b - 1][c - 1];
            const GLenum fe   = form_tag[0    ][c - 1];
//          GLenum form = form_tag[c - 1];
            GLenum type = type_tag[b - 1];
            GLuint o;

            o = text_P->get();

            if (o == 0)
            {
                purge();
                o = text_P->get();
            }

            if (debug)
            {
                glPixelTransferf(GL_RED_SCALE,   color[P->lod()][0] / 255.0);
                glPixelTransferf(GL_GREEN_SCALE, color[P->lod()][1] / 255.0);
                glPixelTransferf(GL_BLUE_SCALE,  color[P->lod()][2] / 255.0);
            }

            if (o)
            {
                glBindTexture(GL_TEXTURE_2D, o);

                // Initialize the texture object.

//              glTexSubImage2D(GL_TEXTURE_2D, 0, -1, -1, s + 2, s + 2, fe, type, D);

                glTexImage2D(GL_TEXTURE_2D, 0, fi, s + 2, s + 2, 1, fe, type, D);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            if (debug)
            {
                glPixelTransferf(GL_RED_SCALE,   1.0f);
                glPixelTransferf(GL_GREEN_SCALE, 1.0f);
                glPixelTransferf(GL_BLUE_SCALE,  1.0f);
            }

            // Assign the texture to the page.

            if (o)
            {
                P->assign(o);
                used(P);
            }
        }
        else
            P->ignore();

        // Release the data buffer.

        free(D);

        return true;
    }
    return false;
}

void uni::geomap::purge()
{
    // If we're over the loaded limit, eject a page.

    if (!LRU.empty())
    {
        GLuint o = LRU.back()->remove();

//      if (o)
        {
            text_P->put(o);
            LRU.pop_back();
//          printf("purged %s %d\n", name.c_str(), o);
        }
//      else printf("purged NOTHING A %s\n", name.c_str());
    }
    else printf("purged NOTHING B %s\n", name.c_str());
}

void uni::geomap::used(page *P)
{
    // Move P to the front of the LRU list;

    LRU.remove(P);
    LRU.push_front(P);
}

//-----------------------------------------------------------------------------

void uni::geomap::draw(const double *V,
                       const double *p, int w, int h)
{
    loaded();

    page::count = 0;

    if (P)
    {
        double r[3];

        vector_to_sphere(r, p[0], p[1], p[2]);

        int d0 = int(floor(log2((r[2] - r0) / k)));
        
        d0 = std::max(d0, 0);
        d0 = std::min(d0, d);

        // HACK
//      d0 = std::max(d0, 5);

        int d1 = d0 + 4;

        P->prep(r, d0, d1);
        P->draw(*this, w, h, d0, d1);
    }

//  printf("%d %s\n", page::count, name.c_str());

    glActiveTextureARB(GL_TEXTURE1);
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void uni::geomap::wire()
{
    if (P) P->wire();
}

//-----------------------------------------------------------------------------
