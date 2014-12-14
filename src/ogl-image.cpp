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

#include <cassert>
#include <cstring>

#include <ogl-opengl.hpp>
#include <ogl-image.hpp>

//-----------------------------------------------------------------------------

ogl::image::image(GLsizei w, GLsizei h,
                  GLenum T, GLenum fi, GLenum fe, GLenum t) :
    target(T), object(0), formint(fi), formext(fe), type(t), w(w), h(h)
{
    p = new GLubyte[w * h * 4];

    memset(p, 0, w * h * 4);

    init();
}

ogl::image::~image()
{
    fini();

    delete [] p;

    p = 0;
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
}

void ogl::image::zero()
{
    memset(p, 0, w * h * 4);

    bind();
    {
        glTexSubImage2D(target, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, p);
    }
    free();
}

//-----------------------------------------------------------------------------

void ogl::image::bind(GLenum unit) const
{
    ogl::bind_texture(target, unit, object);
}

void ogl::image::free(GLenum unit) const
{
}

void ogl::image::draw() const
{
    GLfloat s = (target == GL_TEXTURE_RECTANGLE) ? w : 1.f;
    GLfloat t = (target == GL_TEXTURE_RECTANGLE) ? h : 1.f;

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(target);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        glDisable(GL_BLEND);

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
    if (ogl::context)
    {
        assert(object == 0);
        assert(p);

        // Create a new texture object.

        glGenTextures(1, &object);

        bind();
        {
            glTexImage2D(target, 0, formint, w, h, 0, formext, type, p);

            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        free();
    }
}

void ogl::image::fini()
{
    if (ogl::context)
    {
        assert(object);
        assert(p);

        // Delete the texture object.

        glDeleteTextures(1, &object);
        object = 0;
    }
}

//-----------------------------------------------------------------------------
