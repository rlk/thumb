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

#include "ogl-texture.hpp"
#include "app-serial.hpp"
#include "app-data.hpp"

//-----------------------------------------------------------------------------

#define POT(n) (((n) & ((n) - 1)) == 0)

//-----------------------------------------------------------------------------

ogl::texture::texture(std::string name) :
    name   (name),
    object (0),
    target (GL_TEXTURE_2D),
    filter (GL_LINEAR),
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

        width     = GLsizei(png_get_image_width (rp, ip));
        height    = GLsizei(png_get_image_height(rp, ip));
        GLsizei b = GLsizei(png_get_bit_depth   (rp, ip)) / 8;
        GLsizei c = GLsizei(png_get_channels    (rp, ip));

        target  = GL_TEXTURE_2D;
        border  = 0;
        intform = form_tag[b - 1][c - 1];
        extform = form_tag[0    ][c - 1];
        type    = type_tag[b - 1];

        if      ( POT(width - 2) &&  POT(height - 2))
            border = 1;
        else if (!POT(width    ) || !POT(height    ))
            target = GL_TEXTURE_RECTANGLE_ARB;

        // Switch to compressed textures, as requested.

        if (do_texture_compression)
        {
            if (intform == GL_RGB)
                intform =  GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            if (intform == GL_RGBA)
                intform =  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        }

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            GLsizei stride = (width + border * 2) * b * c;

            GLubyte *p = new GLubyte[height * stride];

            // Initialize the texture object.

            ogl::bind_texture(target, GL_TEXTURE0, object);

            if (target == GL_TEXTURE_2D)
            {
                glTexParameteri(target, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
                glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
                                        GL_LINEAR_MIPMAP_LINEAR);
            }

            if (ogl::has_anisotropic)
                glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                              ogl::max_anisotropy);

            // Copy all rows to the new texture.

            for (GLsizei i = 0, j = height - 1; j >= 0; ++i, --j)
                memcpy(p + stride * i, bp[j], stride);

            glTexImage2D(target, 0, intform, width, height,
                         border, extform, type, p);

            delete [] p;

            OGLCK();
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, 0);
}

//-----------------------------------------------------------------------------

static GLenum wrap_key(const std::string& name)
{
    if (!name.empty())
    {
        if (name == "s") return GL_TEXTURE_WRAP_S;
        if (name == "t") return GL_TEXTURE_WRAP_T;
        if (name == "r") return GL_TEXTURE_WRAP_R;
    }
    return 0;
}

static GLuint wrap_val(const std::string& name)
{
    if (!name.empty())
    {
        if (name == "repeat")          return GL_REPEAT;
        if (name == "clamp")           return GL_CLAMP;
        if (name == "clamp-to-edge")   return GL_CLAMP_TO_EDGE;
        if (name == "clamp-to-border") return GL_CLAMP_TO_BORDER;
    }
    return 0;
}

static GLenum filter_key(const std::string& name)
{
    if (!name.empty())
    {
        if (name == "min") return GL_TEXTURE_MIN_FILTER;
        if (name == "mag") return GL_TEXTURE_MAG_FILTER;
    }
    return 0;
}

static GLint filter_val(const std::string& name)
{
    if (!name.empty())
    {
        if (name == "nearest")                return GL_NEAREST;
        if (name == "linear")                 return GL_LINEAR;
        if (name == "nearest-mipmap-nearest") return GL_NEAREST_MIPMAP_NEAREST;
        if (name == "linear-mipmap-nearest")  return GL_LINEAR_MIPMAP_LINEAR;
        if (name == "nearest-mipmap-linear")  return GL_NEAREST_MIPMAP_NEAREST;
        if (name == "linear-mipmap-linear")   return GL_LINEAR_MIPMAP_LINEAR;
    }
    return 0;
}

//-----------------------------------------------------------------------------

void ogl::texture::load_xml(std::string name)
{
    // Convert the image name to an XML parameter file name.

    std::string path(name, 0, name.rfind("."));

    path.append(".xml");

    // Attempt to load the XML file.

    try
    {
        app::file file(path.c_str());
        app::node   root;
        app::node   node;

        if (app::node p = file.get_root().find("texture"))
        {
            // Parse and apply wrap modes.

            for (app::node n = p.find("wrap"); n; n = p.next(n, "wrap"))
            {
                GLenum key = wrap_key(n.get_s("axis"));
                GLenum val = wrap_val(n.get_s("value"));

                if (key && val) glTexParameteri(target, key, val);
            }

            // Parse and apply filter modes.

            for (app::node n = p.find("filter"); n; n = p.next(n, "filter"))
            {
                GLenum key = filter_key(n.get_s("type"));
                GLenum val = filter_val(n.get_s("value"));

                if (key && val) glTexParameteri(target, key, val);
            }
        }
    }
    catch (app::find_error& e)
    {
    }
}

void ogl::texture::load_img(std::string name)
{
    // Load and parse the data file.

    size_t      len;
    const void *buf = ::data->load(name, &len);

    if (buf) load_png(buf, len);

    ::data->free(name);
}

//-----------------------------------------------------------------------------
/*
void ogl::texture::param_i(GLenum key, GLint val) const
{
    bind();
    {
        glTexParameteri(target, key, val);
    }
    free();
}
*/
//-----------------------------------------------------------------------------

void ogl::texture::bind(GLenum unit) const
{
    ogl::bind_texture(target, unit, object);
}

void ogl::texture::free(GLenum unit) const
{
}

//-----------------------------------------------------------------------------
/*
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
*/
//-----------------------------------------------------------------------------

void ogl::texture::init()
{
    std::string path = "texture/" + name;

    glGenTextures(1, &object);

    load_img(path);
    load_xml(path);

    OGLCK();
}

void ogl::texture::fini()
{
    glDeleteTextures(1, &object);
    object = 0;

    OGLCK();
}

//-----------------------------------------------------------------------------
