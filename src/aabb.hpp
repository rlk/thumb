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

#ifndef AABB_HPP
#define AABB_HPP

#include "opengl.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class aabb
    {
        GLfloat a[3];
        GLfloat z[3];

    public:

        aabb();

        void merge(const GLfloat *);
        void merge(const aabb&);

        GLfloat dist(const GLfloat *,
                     const GLfloat *);
        bool    test(const GLfloat *, int, 
                     const GLfloat *, int&);

        GLfloat length(int i) const { return z[i] - a[i]; }
    };
}

//-----------------------------------------------------------------------------

#endif
