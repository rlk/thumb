//  Copyright (C) 2009 Robert Kooima
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

#ifndef OGL_SHADOW_HPP
#define OGL_SHADOW_HPP

#include "ogl-process.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class frame;
    class binding;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class shadow : public process
    {
        ogl::frame *buff;

        int index;

    public:

        shadow(int);
       ~shadow();

        void bind_frame() const;
        void free_frame() const;
        void bind(GLenum) const;
    };
}

//-----------------------------------------------------------------------------

#endif
