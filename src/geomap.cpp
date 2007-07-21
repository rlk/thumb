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
#include "matrix.hpp"

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

uni::tile::tile(std::string& name, int w, int h, int s,
                int x, int y, int d, double L, double R, double B, double T) :
    state(dead_state), d(d), object(0)
{
    int S = s << d;

    // Compute the mipmap file indices.

    i = y / S;
    j = x / S;

    // Compute the texture boundries.

    this->L = (L + (R - L) * (x    ) / w);
    this->R = (L + (R - L) * (x + S) / w);
    this->T = (T + (B - T) * (y    ) / h);
    this->B = (T + (B - T) * (y + S) / h);

    // Create subtiles as necessary.

    P[0] = 0;
    P[1] = 0;
    P[2] = 0;
    P[3] = 0;

    if (d > 0)
    {
        const int X = (x + S / 2);
        const int Y = (y + S / 2);

        /* Has children? */ P[0] = new tile(name, w, h, s, x, y, d-1, L, R, B, T);
        if (         Y < h) P[1] = new tile(name, w, h, s, x, Y, d-1, L, R, B, T);
        if (X < w         ) P[2] = new tile(name, w, h, s, X, y, d-1, L, R, B, T);
        if (X < w && Y < h) P[3] = new tile(name, w, h, s, X, Y, d-1, L, R, B, T);
    }

    // Test loads

    if (d == 0 && i == 0x0c && j == 0x0d) object = load_png(name, d, i, j);
    if (d == 0 && i == 0x0c && j == 0x0e) object = load_png(name, d, i, j);
    if (d == 1 && i == 0x06 && j == 0x07) object = load_png(name, d, i, j);
    if (d == 7 && i == 0x00 && j == 0x00) object = load_png(name, d, i, j);
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

void uni::tile::draw(int x, int y, int w, int h)
{
    if (object)
    {
        double kt = +1.0 / (R - L);
        double kp = +1.0 / (T - B);
        double dt = -L * kt;
        double dp = -B * kp;

        glActiveTextureARB(GL_TEXTURE1);
        {
            glMatrixMode(GL_TEXTURE);
            {
                glLoadIdentity();
                glTranslated(dt, dp, 0.0);
                glScaled    (kt, kp, 1.0);
            }
            glMatrixMode(GL_MODELVIEW);

            glBindTexture(GL_TEXTURE_2D, object);
        }
        glActiveTextureARB(GL_TEXTURE0);

        glRecti(x, y, w, h);

        glActiveTextureARB(GL_TEXTURE1);
        {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTextureARB(GL_TEXTURE0);
    }

    if (P[0]) P[0]->draw(x, y, w, h);
    if (P[1]) P[1]->draw(x, y, w, h);
    if (P[2]) P[2]->draw(x, y, w, h);
    if (P[3]) P[3]->draw(x, y, w, h);
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name,
                    int w, int h, int c, int b, int s,
                    double L, double R, double B, double T)
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

    P = new tile(name, w, h, s, 0, 0, d, L, R, B, T);
}

uni::geomap::~geomap()
{
    if (P) delete P;
}

void uni::geomap::draw(int x, int y, int w, int h)
{
    if (P) P->draw(x, y, w, h);
}

//-----------------------------------------------------------------------------
