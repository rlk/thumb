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

#ifndef IMGDATA_HPP
#define IMGDATA_HPP

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class imgdata
    {
        GLuint object;

        GLubyte *p;
        GLsizei  w;
        GLsizei  h;
        GLsizei  b;

    public:

        imgdata(GLsizei, GLsizei, GLsizei);
       ~imgdata();

        void blit(const GLubyte *, GLsizei, GLsizei, GLsizei, GLsizei);

        void bind() const;
        void free() const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
