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

#ifndef OGL_IMAGE_HPP
#define OGL_IMAGE_HPP

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class image
    {
        GLenum target;
        GLuint object;
        GLenum formint;
        GLenum formext;
        GLenum type;

        GLvoid *p;
        GLsizei w;
        GLsizei h;

    public:

        image(GLsizei, GLsizei,
              GLenum=GL_TEXTURE_2D,
              GLenum=GL_RGBA8,
              GLenum=GL_RGBA,
              GLenum=GL_UNSIGNED_BYTE);
       ~image();

        void blit(const GLvoid *, GLsizei, GLsizei, GLsizei, GLsizei);
        void zero();

        void bind(GLenum=GL_TEXTURE0) const;
        void free(GLenum=GL_TEXTURE0) const;
        void draw()                   const;

        void init();
        void fini();

        GLsizei get_w() const { return w;      }
        GLsizei get_h() const { return h;      }
        GLuint  get_o() const { return object; }
    };
}

//-----------------------------------------------------------------------------

#endif
