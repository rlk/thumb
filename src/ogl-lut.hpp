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

#ifndef OGL_LUT_HPP
#define OGL_LUT_HPP

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class lut
    {
        GLenum target;
        GLuint object;
        GLenum formint;
        GLenum formext;
        GLenum type;

        GLsizei w;

    public:

        lut(GLsizei, GLenum=GL_TEXTURE_1D,
                     GLenum=GL_RGBA8,
                     GLenum=GL_RGBA,
                     GLenum=GL_UNSIGNED_BYTE);
        ~lut();

        void blit(const GLvoid *, GLsizei, GLsizei) const;

        void bind(GLenum=GL_TEXTURE0) const;
        void free(GLenum=GL_TEXTURE0) const;
    };
}

//-----------------------------------------------------------------------------

#endif
