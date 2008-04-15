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

#ifndef LUT_HPP
#define LUT_HPP

#include "ogl-opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class lut
    {
        GLsizei n;

        GLenum target;
        GLuint texture;

    public:

        lut(GLsizei, GLenum=GL_TEXTURE_1D,
                     GLenum=GL_RGBA8,
                     GLenum=GL_RGBA);
        ~lut();

        void bind(GLenum=GL_TEXTURE0) const;
        void free(GLenum=GL_TEXTURE0) const;
    };
}

//-----------------------------------------------------------------------------

#endif
