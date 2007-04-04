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

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl 
{
    class image
    {
        GLenum target;
        GLenum format;
        GLuint texture;

        GLsizei w;
        GLsizei h;

    public:

        image(GLenum=GL_TEXTURE_2D, GLenum=GL_RGBA8, GLsizei=2, GLsizei=2);
        image(std::string);
       ~image();

        void bind(GLenum=GL_TEXTURE0) const;
        void free(GLenum=GL_TEXTURE0) const;
        void draw()                   const;
        void null()                   const;

        void load(std::string, GLenum=GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        void save(std::string, GLenum=GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
        void snap(             GLenum=GL_TEXTURE_CUBE_MAP_POSITIVE_Z);

        GLsizei get_w() const { return w;       }
        GLsizei get_h() const { return h;       }
        GLuint  get_o() const { return texture; }
    };
}

//-----------------------------------------------------------------------------

#endif
