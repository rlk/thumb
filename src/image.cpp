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

#include <png.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "image.hpp"

//-----------------------------------------------------------------------------

ogl::image::image(GLenum t, GLenum f, GLsizei w, GLsizei h) :
    target(t), format(f), w(w), h(h)
{
    // Generate the texture object.

    glGenTextures(1, &texture);
    glBindTexture(t,  texture);

    // Zero the buffer(s).

    if (target == GL_TEXTURE_CUBE_MAP)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, f, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
    else
        glTexImage2D(t, 0, f, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Set some useful default state.

    if (t == GL_TEXTURE_2D)
    {
        // TODO: Confirm GENERATE_MIPMAP support.

        glTexParameteri(t, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(t, 0);
    OGLCK();
}

ogl::image::image(std::string filename) :
    target(GL_TEXTURE_2D), format(GL_RGBA8), w(2), h(2)
{
    load(filename);
}

ogl::image::~image()
{
    glDeleteTextures(1, &texture);
    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::image::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, texture);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

void ogl::image::free(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
    OGLCK();
}

void ogl::image::null() const
{
    glBindTexture(target, texture);
    {
        glTexImage2D(target, 0, format, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    }
    glBindTexture(target, 0);

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::image::draw() const
{
    GLint v[4];

    glPushAttrib(GL_ENABLE_BIT);
    {
        // Set up a one-to-one model-view-projection.

        glGetIntegerv(GL_VIEWPORT, v);

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
            glOrtho(v[0], v[2], v[1], v[3], 0, 1);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        // Set up to render the unlit texture.

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glEnable(target);

        bind();

        // Draw a screen-filling rectangle.

        glBegin(GL_QUADS);
        {
            GLfloat x = GLfloat(w);
            GLfloat y = GLfloat(h);

            GLfloat s = (target == GL_TEXTURE_2D) ? 1 : x;
            GLfloat t = (target == GL_TEXTURE_2D) ? 1 : y;

            glTexCoord3f(0, 0, 1); glVertex2f(0, 0);
            glTexCoord3f(s, 0, 1); glVertex2f(x, 0);
            glTexCoord3f(s, t, 1); glVertex2f(x, y);
            glTexCoord3f(0, t, 1); glVertex2f(0, y);
        }
        glEnd();

        // Restore the transform.

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    glPopAttrib();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::image::load(std::string filename, GLenum face)
{
    FILE       *fp = 0;
    png_structp rp = 0;
    png_infop   ip = 0;
    png_bytep  *bp = 0;

    // Initialize all PNG import data structures.

    if (!(fp = fopen(filename.c_str(), "rb")))
        throw std::runtime_error("Failure opening PNG file for reading");

    if (!(rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG read structure");

    if (!(ip = png_create_info_struct(rp)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(rp)) == 0)
    {
        // Read the PNG header.

        png_init_io (rp, fp);
        png_read_png(rp, ip, PNG_TRANSFORM_STRIP_16 |
                             PNG_TRANSFORM_PACKING, 0);
        
        // Extract image properties.

        w = GLsizei(png_get_image_width (rp, ip));
        h = GLsizei(png_get_image_height(rp, ip));

        if (w & (w - 1)) target = GL_TEXTURE_RECTANGLE_ARB;
        if (h & (h - 1)) target = GL_TEXTURE_RECTANGLE_ARB;

        // Select the texture format.

        GLsizei b = 4;

        switch (png_get_color_type(rp, ip))
        {
        case PNG_COLOR_TYPE_GRAY:
            b = 1; format = GL_LUMINANCE;       break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            b = 2; format = GL_LUMINANCE_ALPHA; break;
        case PNG_COLOR_TYPE_RGB:
            b = 3; format = GL_RGB;             break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            b = 4; format = GL_RGBA;            break;
        }

        // Read the pixel data.

        if ((bp = png_get_rows(rp, ip)))
        {
            // Allocate a pixel buffer and copy pixels there.

            GLubyte *p = new GLubyte[w * h * b];

            for (GLsizei i = 0; i < h; ++i)
                memcpy(p + i * w * b, bp[h - i - 1], w * b);

            // Load the image data as OpenGL texture.

            glBindTexture(target, texture);
            {
                if (target == GL_TEXTURE_CUBE_MAP)
                    glTexImage2D(face,   0, format, w, h, 0,
                                 format, GL_UNSIGNED_BYTE, p);
                else
                    glTexImage2D(target, 0, format, w, h, 0,
                                 format, GL_UNSIGNED_BYTE, p);
            }
            glBindTexture(target, 0);

            delete [] p;
        }
    }

    // Release all resources.

    png_destroy_read_struct(&rp, &ip, 0);
    fclose(fp);
}

void ogl::image::save(std::string filename, GLenum face)
{
    FILE       *fp = 0;
    png_structp wp = 0;
    png_infop   ip = 0;
    png_bytep  *bp = 0;

    // Initialize all PNG export data structures.

    if (!(fp = fopen(filename.c_str(), "wb")))
        throw std::runtime_error("Failure opening PNG file for writing");

    if (!(wp = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG write structure");

    if (!(ip = png_create_info_struct(wp)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(wp)) == 0)
    {
        // Initialize the PNG header.

        png_init_io (wp, fp);
        png_set_compression_level(wp, 9);
        png_set_IHDR(wp, ip, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
                                      PNG_INTERLACE_NONE,
                                      PNG_COMPRESSION_TYPE_DEFAULT,
                                      PNG_FILTER_TYPE_DEFAULT);

        // Allocate the row pointer array.

        if ((bp = (png_bytep *) png_malloc(wp, h * sizeof (png_bytep))))
        {
            // Allocate a pixel buffer and copy pixels there.

            GLubyte *p = new GLubyte[w * h * 4];

            glBindTexture(target, texture);
            {
                if (target == GL_TEXTURE_CUBE_MAP)
                    glGetTexImage(face,   0, GL_RGBA, GL_UNSIGNED_BYTE, p);
                else
                    glGetTexImage(target, 0, GL_RGBA, GL_UNSIGNED_BYTE, p);
            }
            glBindTexture(target, 0);

            // Initialize the row pointers.

            for (GLsizei i = 0; i < h; ++i)
                bp[h - i - 1] = (png_bytep) (p + i * w * 4);

            // Write the PNG image file.

            png_set_rows  (wp, ip, bp);
            png_write_info(wp, ip);
            png_write_png (wp, ip, 0, 0);

            delete [] p;
            png_free(wp, bp);
        }
    }

    // Release all resources.

    png_destroy_write_struct(&wp, &ip);
    fclose(fp);
}

void ogl::image::snap(GLenum face)
{
    GLint v[4];

    // Determine the image size and allocate a buffer.

    glGetIntegerv(GL_VIEWPORT, v);

    w = GLsizei(v[2]);
    h = GLsizei(v[3]);

    GLubyte *p = new GLubyte[w * h * 4];

    // Copy the current frame buffer.
    
    format = GL_RGBA;

    glReadPixels(0, 0, w, h, format, GL_UNSIGNED_BYTE, p);

    if (target == GL_TEXTURE_CUBE_MAP)
        glTexImage2D(face,   0, format, w, h, 0,
                     format, GL_UNSIGNED_BYTE, p);
    else
        glTexImage2D(target, 0, format, w, h, 0,
                     format, GL_UNSIGNED_BYTE, p);

    delete [] p;
}

//-----------------------------------------------------------------------------

