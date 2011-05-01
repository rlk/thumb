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

#ifndef OGL_BUFFER_HPP
#define OGL_BUFFER_HPP

#include <ogl-opengl.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class buffer
    {
        GLuint  o;   // Buffer object
        GLenum  t;   // Target
        GLsizei s;   // Size
        GLenum  p;   // Usage policy

    public:

        buffer(GLenum, GLsizei, GLenum=GL_DYNAMIC_DRAW_ARB);
       ~buffer();

        void  bind()       const;
        void  bind(GLenum) const;
        void  free()       const;
        void  free(GLenum) const;
        void  zero()       const;

        void *rmap() const;
        void *wmap() const;
        void *amap() const;
        void  umap() const;
    };
}

//-----------------------------------------------------------------------------

#endif
