//  Copyright (C) 2007-2011 Robert Kooima
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

#include <ogl-texture.hpp>
#include <app-file.hpp>
#include <app-conf.hpp>
#include <app-data.hpp>

//-----------------------------------------------------------------------------

#define POT(n) (((n) & ((n) - 1)) == 0)

//-----------------------------------------------------------------------------

ogl::texture::texture(std::string name) : name(name), object(0), w(0), h(0), c(0)
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

static void downsample(GLsizei W, GLsizei H, GLsizei C, std::vector<GLubyte>& P)
{
    const GLsizei w = W / 2;
    const GLsizei h = H / 2;

    for         (GLsizei i = 0; i < h; i++)
        for     (GLsizei j = 0; j < w; j++)
            for (GLsizei k = 0; k < C; k++)
            {
                GLuint b = GLuint(P[((2 * i + 0) * W + (2 * j + 0)) * C + k])
                         + GLuint(P[((2 * i + 0) * W + (2 * j + 1)) * C + k])
                         + GLuint(P[((2 * i + 1) * W + (2 * j + 0)) * C + k])
                         + GLuint(P[((2 * i + 1) * W + (2 * j + 1)) * C + k]);

                P[(i * w + j) * C + k] = GLubyte(b / 4);
            }
}

//-----------------------------------------------------------------------------

void ogl::texture::load_png(const void *buf, size_t len, std::vector<GLubyte>& p)
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

        png_read_png(rp, ip, PNG_TRANSFORM_EXPAND   |
                             PNG_TRANSFORM_PACKING  |
                             PNG_TRANSFORM_STRIP_16 |
                             PNG_TRANSFORM_SWAP_ENDIAN, 0);

        // Extract image properties.

        w = GLsizei(png_get_image_width (rp, ip));
        h = GLsizei(png_get_image_height(rp, ip));
        c = GLsizei(png_get_channels    (rp, ip));

        p.reserve(w * h * c);

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))

            for (GLsizei i = 0, j = h - 1; i < h; ++i, --j)
                memcpy(&p.front() + (w * c) * i, bp[j], (w * c));
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

void ogl::texture::load_opt(std::string name, std::map<int, vec4>& scale)
{
    // Convert the image name to an XML parameter file name.

    std::string path(name, 0, name.rfind("."));

    path.append(".xml");

    // Attempt to load the XML file.

    if (::data->find(path))
    {
        app::file file(path);

        if (app::node p = file.get_root().find("texture"))
        {
            // Parse and apply options.

            for (app::node n = p.find("option"); n; n = p.next(n, "option"))
            {
                if ("scale" == n.get_s("name"))
                {
                    int    l = n.get_i("level", 0);
                    double r = n.get_f("red",   1);
                    double g = n.get_f("green", 1);
                    double b = n.get_f("blue",  1);
                    double a = n.get_f("alpha", 1);

                    scale[l] = vec4(r, g, b, a);
                }
            }
        }
    }
}

void ogl::texture::load_prm(std::string name)
{
    // Convert the image name to an XML parameter file name.

    std::string path(name, 0, name.rfind("."));

    path.append(".xml");

    // Attempt to load the XML file.

    if (::data->find(path))
    {
        app::file file(path);

        if (app::node p = file.get_root().find("texture"))
        {
            // Parse and apply wrap modes.

            for (app::node n = p.find("wrap"); n; n = p.next(n, "wrap"))
            {
                GLenum key = wrap_key(n.get_s("axis"));
                GLenum val = wrap_val(n.get_s("value"));

                if (key && val) glTexParameteri(GL_TEXTURE_2D, key, val);
            }

            // Parse and apply filter modes.

            for (app::node n = p.find("filter"); n; n = p.next(n, "filter"))
            {
                GLenum key = filter_key(n.get_s("type"));
                GLenum val = filter_val(n.get_s("value"));

                if (key && val) glTexParameteri(GL_TEXTURE_2D, key, val);
            }
        }
    }
}

void ogl::texture::load_img(std::string name, std::map<int, vec4>& scale)
{
    std::vector<GLubyte> pixels;

    // Load and parse the data file.

    size_t      len;
    const void *buf = ::data->load(name, &len);

    if (buf) load_png(buf, len, pixels);

    ::data->free(name);

    // Initialize the OpenGL texture object.

    GLenum f = GL_RGBA;

    switch (c)
    {
        case 1: f = GL_LUMINANCE;
        case 2: f = GL_LUMINANCE_ALPHA;
        case 3: f = GL_RGB;
    }

    ogl::bind_texture(GL_TEXTURE_2D, GL_TEXTURE0, object);

    GLubyte *p = &pixels.front();
    GLsizei ww = w;
    GLsizei hh = h;

    // Enumerate the mipmap levels.

    for (GLint l = 0; ww > 0 && hh > 0; l++)
    {
        std::map<int, vec4>::iterator it;

        // Set the scale for this mipmap.

        if ((it = scale.find(l)) == scale.end())
        {
            glPixelTransferf(GL_RED_SCALE,   1.f);
            glPixelTransferf(GL_GREEN_SCALE, 1.f);
            glPixelTransferf(GL_BLUE_SCALE,  1.f);
            glPixelTransferf(GL_ALPHA_SCALE, 1.f);
        }
        else
        {
            glPixelTransferf(GL_RED_SCALE,   GLfloat(it->second[0]));
            glPixelTransferf(GL_GREEN_SCALE, GLfloat(it->second[1]));
            glPixelTransferf(GL_BLUE_SCALE,  GLfloat(it->second[2]));
            glPixelTransferf(GL_ALPHA_SCALE, GLfloat(it->second[3]));
        }

        // Copy the pixels.

        glTexImage2D(GL_TEXTURE_2D, l, f, ww, hh, 0, f, GL_UNSIGNED_BYTE, p);

        // Prepare for the next mipmap level.

        downsample(ww, hh, c, pixels);
        ww /= 2;
        hh /= 2;
    }

    // Initialize the default texture parameters.

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    if (ogl::has_anisotropic)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                                                ogl::max_anisotropy);
}

//-----------------------------------------------------------------------------

void ogl::texture::bind(GLenum unit) const
{
    ogl::bind_texture(GL_TEXTURE_2D, unit, object);
}

void ogl::texture::free(GLenum unit) const
{
}

//-----------------------------------------------------------------------------

void ogl::texture::init()
{
    std::map<int, vec4> scale;

    std::string path = "texture/" + name;

    glGenTextures(1, &object);

    load_opt(path, scale);
    load_img(path, scale);
    load_prm(path);
}

void ogl::texture::fini()
{
    glDeleteTextures(1, &object);
    object = 0;
}

//-----------------------------------------------------------------------------
