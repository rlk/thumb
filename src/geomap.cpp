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
#include "program.hpp"
#include "matrix.hpp"
#include "view.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

static bool   debug_init = false;
static GLuint debug_text[8];

static GLubyte debug_color[8][4] = {
    { 0xFF, 0x00, 0x00, 0xFF },
    { 0x00, 0xFF, 0x00, 0xFF },
    { 0x00, 0x00, 0xFF, 0xFF },
    { 0xFF, 0xFF, 0x00, 0xFF },
    { 0x00, 0xFF, 0xFF, 0xFF },
    { 0xFF, 0x00, 0xFF, 0xFF },
    { 0x7F, 0x7F, 0x7F, 0xFF },
    { 0xFF, 0xFF, 0xFF, 0xFF },
};

static void setup_debug()
{
    if (debug_init == false)
    {
        debug_init = true;

        glGenTextures(8, debug_text);

        for (int i = 0; i < 8; ++i)
        {
            GLubyte p[16];

            for (int j = 0; j < 16; j += 4)
            {
                p[j + 0] = debug_color[i][0];
                p[j + 1] = debug_color[i][1];
                p[j + 2] = debug_color[i][2];
                p[j + 3] = debug_color[i][3];
            }

            glBindTexture(GL_TEXTURE_2D, debug_text[i]);
            glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0,
                          GL_RGBA, GL_UNSIGNED_BYTE, p);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

//-----------------------------------------------------------------------------

static GLuint load_png(std::string base, int d, int i, int j)
{
    GLuint o = 0;

    // Construct the file name.

    std::ostringstream name;

    name << base
         << "-" << std::hex << std::setfill('0') << std::setw(2) << d
         << "-" << std::hex << std::setfill('0') << std::setw(2) << i
         << "-" << std::hex << std::setfill('0') << std::setw(2) << j << ".png";

    // Initialize all PNG import data structures.

    png_structp rp = 0;
    png_infop   ip = 0;
    png_bytep  *bp = 0;
    FILE       *fp = 0;

    if (!(fp = fopen(name.str().c_str(), "rb")))
        throw std::runtime_error("Failure opening PNG file");

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG read structure");

    if (!(ip = png_create_info_struct(rp)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        GLenum type_tag[2] = {
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_SHORT
        };
        GLenum form_tag[2][4] = {
            { GL_LUMINANCE,   GL_LUMINANCE_ALPHA,     GL_RGB,   GL_RGBA   },
            { GL_LUMINANCE16, GL_LUMINANCE16_ALPHA16, GL_RGB16, GL_RGBA16 },
        };

        // Read the PNG header.

        png_init_io (rp, fp);
        png_read_png(rp, ip, PNG_TRANSFORM_PACKING |
                             PNG_TRANSFORM_SWAP_ENDIAN, 0);
        
        // Extract image properties.

        GLsizei w = GLsizei(png_get_image_width (rp, ip));
        GLsizei h = GLsizei(png_get_image_height(rp, ip));
        GLsizei b = GLsizei(png_get_bit_depth   (rp, ip)) / 8;
        GLsizei c = 1;
        GLsizei d = 0;

        if (w & (w - 1)) d = 1;
        if (h & (h - 1)) d = 1;

        switch (png_get_color_type(rp, ip))
        {
        case PNG_COLOR_TYPE_GRAY:       c = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: c = 2; break;
        case PNG_COLOR_TYPE_RGB:        c = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  c = 4; break;
        default: throw std::runtime_error("Unsupported PNG color type");
        }

        GLenum fi   = form_tag[b - 1][c - 1];
        GLenum fe   = form_tag[0    ][c - 1];
        GLenum type = type_tag[b - 1];

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            glGenTextures(1, &o);

            // Initialize the texture object.

            glBindTexture(GL_TEXTURE_2D, o);
            glTexImage2D (GL_TEXTURE_2D, 0, fi, w, h, d, fe, type, 0);

            OGLCK();

            // Copy all rows to the new texture.

            for (GLsizei i = 0, j = h - 1; j >= 0; ++i, --j)
                glTexSubImage2D(GL_TEXTURE_2D, 0, -d, i - d, w, 1,
                                fe, type, bp[j]);

            OGLCK();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, 0);
    fclose(fp);

    return o;
}

//-----------------------------------------------------------------------------

uni::tile::tile(std::string& name,
                int w, int h, int s,
                int x, int y, int d,
                double r0, double r1,
                double _L, double _R, double _B, double _T) :
    state(dead_state), d(d), object(0), hint(0)
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

    // Create subtiles as necessary.

    P[0] = 0;
    P[1] = 0;
    P[2] = 0;
    P[3] = 0;

    if (d > 0)
    {
        const int X = (x + S / 2);
        const int Y = (y + S / 2);

        /* Has children? */ P[0] = new tile(name, w, h, s, x, y, d-1, r0, r1, _L, _R, _B, _T);
        if (         Y < h) P[1] = new tile(name, w, h, s, x, Y, d-1, r0, r1, _L, _R, _B, _T);
        if (X < w         ) P[2] = new tile(name, w, h, s, X, y, d-1, r0, r1, _L, _R, _B, _T);
        if (X < w && Y < h) P[3] = new tile(name, w, h, s, X, Y, d-1, r0, r1, _L, _R, _B, _T);

        // Accumulate the AABBs of the children. */

        b[0] =  std::numeric_limits<double>::max();
        b[1] =  std::numeric_limits<double>::max();
        b[2] =  std::numeric_limits<double>::max();
        b[3] = -std::numeric_limits<double>::max();
        b[4] = -std::numeric_limits<double>::max();
        b[5] = -std::numeric_limits<double>::max();

        for (int k = 0; k < 4; ++k)
            if (P[k])
            {
                b[0] = std::min(b[0], P[k]->b[0]);
                b[1] = std::min(b[1], P[k]->b[1]);
                b[2] = std::min(b[2], P[k]->b[2]);
                b[3] = std::max(b[3], P[k]->b[3]);
                b[4] = std::max(b[4], P[k]->b[4]);
                b[5] = std::max(b[5], P[k]->b[5]);
            }
    }
    else
    {
        // Compute the AABB of this tile.

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
    }

    // Compute the center of this tile.

    c[0] = (b[0] + b[3]) / 2.0;
    c[1] = (b[1] + b[4]) / 2.0;
    c[2] = (b[2] + b[5]) / 2.0;

    area = (R - L) * (T - B);
    size = sqrt(area);

    if (i == 0 && j == 0) printf("%d %f %f\n", d, size, (1 << d) * 10000.0);

    is_visible = false;
    will_draw  = false;

    // Test loads

    object = debug_text[d];

//  if (i == 0 && j == 0) object = load_png(name, d, i, j);
}

uni::tile::~tile()
{
    if (P[3]) delete P[3];
    if (P[2]) delete P[2];
    if (P[1]) delete P[1];
    if (P[0]) delete P[0];

    eject();
}

void uni::tile::ready(GLuint o)
{
    object = o;
    state  = live_state;
}

void uni::tile::eject()
{
    if (object)
        glDeleteTextures(1, &object);

    object = 0;
    state  = dead_state;
}

//-----------------------------------------------------------------------------
/*
static void project(double *r, const double *M, double x, double y, double z)
{
    double v[4] = { x, y, z, 1 };
    double w[4];
    
    mult_mat_vec4(w, M, v);

    double rx = w[0] / w[3];
    double ry = w[1] / w[3];

    r[0] = std::min(r[0], rx);
    r[1] = std::min(r[1], ry);
    r[2] = std::max(r[2], rx);
    r[3] = std::max(r[3], ry);
}

int uni::tile::size(const double *MVP)
{
    double r[4];

    r[0] =  std::numeric_limits<double>::max();
    r[1] =  std::numeric_limits<double>::max();
    r[2] = -std::numeric_limits<double>::max();
    r[3] = -std::numeric_limits<double>::max();

    project(r, MVP, b[0], b[1], b[2]);
    project(r, MVP, b[0], b[1], b[5]);
    project(r, MVP, b[0], b[4], b[2]);
    project(r, MVP, b[0], b[4], b[5]);
    project(r, MVP, b[3], b[1], b[2]);
    project(r, MVP, b[3], b[1], b[5]);
    project(r, MVP, b[3], b[4], b[2]);
    project(r, MVP, b[3], b[4], b[5]);

    int w = int((r[2] - r[0]) * ::view->get_w() / 2);
    int h = int((r[3] - r[1]) * ::view->get_h() / 2);

//  printf("%2d %2d %2d %4d %4d\n", d, i, j, w, h);

    return (w * h);
}
*/
bool uni::tile::value(const double *p)
{
    double bias = 500000.0;
    double v[3];

    v[0] = c[0] - p[0];
    v[1] = c[1] - p[1];
    v[2] = c[2] - p[2];

    double k = sqrt(DOT3(v, v));

//  if (d == 1) printf("%f %f\n", k, (1 << d) * bias);

    return (k < ((1 << d) * bias));
}

bool uni::tile::test(const double *P)
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

bool uni::tile::visible(const double *V)
{
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

//-----------------------------------------------------------------------------

void uni::tile::volume() const
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

void uni::tile::prep(const double *V,
                     const double *p)
{
    is_visible = false;
    will_draw  = false;

    if (visible(V))
    {
        is_visible = true;

        if (value(p))
        {
            will_draw = true;

            if (P[0]) P[0]->prep(V, p);
            if (P[1]) P[1]->prep(V, p);
            if (P[2]) P[2]->prep(V, p);
            if (P[3]) P[3]->prep(V, p);
        }
    }
}

void uni::tile::draw(int w, int h)
{
    if (is_visible)
    {
/*
        if (P[0]) P[0]->draw(w, h);
        if (P[1]) P[1]->draw(w, h);
        if (P[2]) P[2]->draw(w, h);
        if (P[3]) P[3]->draw(w, h);
*/
        bool p0, p1, p2, p3;

        p0 = P[0] && (!P[0]->is_visible || (P[0]->will_draw && P[0]->object));
        p1 = P[1] && (!P[1]->is_visible || (P[1]->will_draw && P[1]->object));
        p2 = P[2] && (!P[2]->is_visible || (P[2]->will_draw && P[2]->object));
        p3 = P[3] && (!P[3]->is_visible || (P[3]->will_draw && P[3]->object));

        bool pp = (p0 && p1 && p2 && p3);

        if (will_draw && !pp)
        {
            if (object)
            {
                // Set up this tile's texture transform.

                double kt = +1.0 / (R - L);
                double kp = +1.0 / (T - B);
                double dt = -L * kt;
                double dp = -B * kp;

                glActiveTextureARB(GL_TEXTURE1);
                {
                    glBindTexture(GL_TEXTURE_2D, object);
                }
                glActiveTextureARB(GL_TEXTURE0);

                // TODO: store this uniform location

                ogl::program::current->uniform("d", dt, dp);
                ogl::program::current->uniform("k", kt, kp);

                // Draw this tile's bounding volume.

                if (w > 0 && h > 0)
                    glRecti(0, 0, w, h);
                else
                    volume();
            }
        }

        if (P[0]) P[0]->draw(w, h);
        if (P[1]) P[1]->draw(w, h);
        if (P[2]) P[2]->draw(w, h);
        if (P[3]) P[3]->draw(w, h);
    }
}

void uni::tile::wire()
{
    if (will_draw)
    {
        volume();

        if (P[0]) P[0]->wire();
        if (P[1]) P[1]->wire();
        if (P[2]) P[2]->wire();
        if (P[3]) P[3]->wire();
    }
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name,
                    int w, int h, int c, int b, int s,
                    double r0, double r1,
                    double L, double R, double B, double T) : name(name)
{
    // Compute the depth of the mipmap pyramid.

    int S = s;
    int d = 0;

    while (S < w || S < h)
    {
        S *= 2;
        d += 1;
    }

    // Generate the mipmap pyramid catalog.

    setup_debug();
    glBindTexture(GL_TEXTURE_2D, 0);

    P = new tile(name, w, h, s, 0, 0, d, r0, r1, L, R, B, T);
}

uni::geomap::~geomap()
{
    if (P) delete P;
}

void uni::geomap::draw(const double *V,
                       const double *p, int w, int h)
{
    if (P)
    {
        P->prep(V, p);
        P->draw(w, h);
    }

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
