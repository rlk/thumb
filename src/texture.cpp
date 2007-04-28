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
#include "data.hpp"

//-----------------------------------------------------------------------------

ogl::texture::texture(std::string name) : name(name),
    target(GL_TEXTURE_2D),
    format(GL_RGBA)
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
        // Read the PNG header.

        png_read_png(rp, ip, PNG_TRANSFORM_STRIP_16 |
                             PNG_TRANSFORM_PACKING, 0);
        
        // Extract image properties.

        GLsizei w = GLsizei(png_get_image_width (rp, ip));
        GLsizei h = GLsizei(png_get_image_height(rp, ip));

        target = GL_TEXTURE_2D;
        format = GL_RGBA;

        if (w & (w - 1)) target = GL_TEXTURE_RECTANGLE_ARB;
        if (h & (h - 1)) target = GL_TEXTURE_RECTANGLE_ARB;

        switch (png_get_color_type(rp, ip))
        {
        case PNG_COLOR_TYPE_GRAY:       format = GL_LUMINANCE;       break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: format = GL_LUMINANCE_ALPHA; break;
        case PNG_COLOR_TYPE_RGB:        format = GL_RGB;             break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  format = GL_RGBA;            break;
        }

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Initialize the texture object.

            glEnable(target);

            glBindTexture(target, object);

            glTexImage2D(target, 0, format, w, h, 0,
                         format, GL_UNSIGNED_BYTE, 0);

            // Copy all rows to the new texture.

            for (GLsizei i = 0, j = h - 1; j >= 0; ++i, --j)
                glTexSubImage2D(target, 0, 0, i, w, 1,
                                format, GL_UNSIGNED_BYTE, bp[j]);
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

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
