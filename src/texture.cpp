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

#include <png.h>

#include "texture.hpp"
#include "app-data.hpp"

//-----------------------------------------------------------------------------

#define POT(n) (((n) & ((n) - 1)) == 0)

//-----------------------------------------------------------------------------

ogl::texture::texture(std::string name, GLenum filter) :
    name   (name),
    object (0),
    target (GL_TEXTURE_2D),
    filter (filter),
    intform(GL_RGBA),
    extform(GL_RGBA),
    type   (GL_UNSIGNED_BYTE)
{
    init();
}

ogl::texture::~texture()
{
    fini();
}

//-----------------------------------------------------------------------------

struct png_user_io
{
    png_bytep  buf;
    png_size_t len;
};

static void png_user_read(png_structp rp, png_bytep buf, png_size_t len)
{
    struct png_user_io *userp = (struct png_user_io *) png_get_io_ptr(rp);

    if (userp->len >= len)
    {
        memcpy(buf, userp->buf, len);
        userp->buf += len;
        userp->len -= len;
    }
    else png_error(rp, "premature end-of-file");
}

//-----------------------------------------------------------------------------

void ogl::texture::load_png(const void *buf, size_t len)
{
    // Initialize all PNG import data structures.

    png_structp rp = 0;
    png_infop   ip = 0;
    png_bytep  *bp = 0;

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG read structure");

    if (!(ip = png_create_info_struct(rp)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Initialize the user-defined IO structure.

    struct png_user_io user;

    user.buf = (png_bytep)  buf;
    user.len = (png_size_t) len;

    png_set_read_fn(rp, &user, png_user_read);

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        GLenum type_tag[2] = {
            GL_UNSIGNED_BYTE,
            GL_UNSIGNED_SHORT
        };
        GLenum form_tag[2][4] = {
            {
                GL_LUMINANCE,
                GL_LUMINANCE_ALPHA,
                GL_RGB,
                GL_RGBA,
            },
            {
                GL_LUMINANCE16,
                GL_LUMINANCE16_ALPHA16,
                GL_RGB16,
                GL_RGBA16,
            },
        };

        // Read the PNG header.

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

        target  = GL_TEXTURE_2D;
        border  = 0;
        intform = form_tag[b - 1][c - 1];
        extform = form_tag[0    ][c - 1];
        type    = type_tag[b - 1];

        if      ( POT(w - 2) &&  POT(h - 2)) border = 1;
        else if (!POT(w    ) || !POT(h    )) target = GL_TEXTURE_RECTANGLE_ARB;

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Initialize the texture object.

            glBindTexture(target, object);

/* TODO: GENERATE_MIPMAP + TexSubImage is SLOW
            if (target == GL_TEXTURE_2D)
                glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
*/
            glTexImage2D(target, 0, intform, w, h, border, extform, type, 0);

            OGLCK();

            // Copy all rows to the new texture.

            for (GLsizei i = 0, j = h - 1; j >= 0; ++i, --j)
                glTexSubImage2D(target,
                                0, -border,
                                i - border,
                                w, 1, extform, type, bp[j]);

            OGLCK();
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, 0);
}

//-----------------------------------------------------------------------------

void ogl::texture::load_img(std::string name)
{
    size_t      len;
    const void *buf = ::data->load(name.c_str(), &len);

    if (buf) load_png(buf, len);

    ::data->free(name);

    // Set some useful default state.

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

    if (border)
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    else
    {
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

//-----------------------------------------------------------------------------

void ogl::texture::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, object);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

void ogl::texture::free(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::texture::draw() const
{
    bind(GL_TEXTURE1);
    {
        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glRecti(-1, -1, +1, +1);

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    free(GL_TEXTURE1);
}

//-----------------------------------------------------------------------------

void ogl::texture::init()
{
    glGenTextures(1, &object);

    load_img(name);

    OGLCK();
}

void ogl::texture::fini()
{
    glDeleteTextures(1, &object);

    OGLCK();
}

//-----------------------------------------------------------------------------
