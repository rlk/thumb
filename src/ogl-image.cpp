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

#include "ogl-opengl.hpp"
#include "ogl-image.hpp"

//-----------------------------------------------------------------------------

ogl::image::image(GLsizei w, GLsizei h,
                  GLenum T, GLenum fi, GLenum fe, GLenum t) :
    target(T), object(0), formint(fi), formext(fe), type(t), p(0), w(w), h(h)
{
    init();
}

ogl::image::~image()
{
    fini();

//  if (p) delete [] p;
}

//-----------------------------------------------------------------------------

void ogl::image::blit(const GLvoid *P, GLsizei X, GLsizei Y,
                                       GLsizei W, GLsizei H)
{
    bind();
    {
        glTexSubImage2D(target, 0, X, Y, W, H, formext, type, P);
    }
    free();

    OGLCK();
}

void ogl::image::zero()
{
    bind();
    {
        glTexImage2D(target, 0, formint, w, h, 0, formext, type, 0);
    }
    free();

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::image::bind(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, object);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::image::free(GLenum unit) const
{
    glActiveTextureARB(unit);
    {
        glBindTexture(target, 0);
    }
    glActiveTextureARB(GL_TEXTURE0);
}

void ogl::image::draw() const
{
    GLfloat s = (target == GL_TEXTURE_RECTANGLE_ARB) ? w : 1.0;
    GLfloat t = (target == GL_TEXTURE_RECTANGLE_ARB) ? h : 1.0;

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(target);
        glDisable(GL_LIGHTING);

        bind(GL_TEXTURE0);
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

            glBegin(GL_QUADS);
            {
                glTexCoord2f(0, 0); glVertex2f(-1.0f, -1.0f);
                glTexCoord2f(s, 0); glVertex2f(+1.0f, -1.0f);
                glTexCoord2f(s, t); glVertex2f(+1.0f, +1.0f);
                glTexCoord2f(0, t); glVertex2f(-1.0f, +1.0f);
            }
            glEnd();

            glMatrixMode(GL_PROJECTION);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glPopMatrix();
            }
        }
        free(GL_TEXTURE0);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

void ogl::image::init()
{
    // Create a new texture object.

    glGenTextures(1,     &object);
    glBindTexture(target, object);

    glTexImage2D(target, 0, formint, w, h, 0, formext, type, p);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
/*
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
*/

    // If we have a buffer, we must be reloading.  It's no longer needed.
    /*
    if (p)
    {
        delete [] p;
        p = 0;
    }
    */

    OGLCK();
}

void ogl::image::fini()
{
    /*
    // Allocate and store a copy of the image in main memory.

    p = new GLubyte[w * h * 4];

    bind();
    {
        glGetTexImage(target, 0, formext, type, p);
    }
    free();
    */

    // Delete the texture object.

    glDeleteTextures(1, &object);
    OGLCK();
}

//-----------------------------------------------------------------------------
