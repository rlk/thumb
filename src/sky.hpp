//  Copyright (C) 2005 Robert Kooima
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

#ifndef SKY_HPP
#define SKY_HPP

#include "solid.hpp"
#include "image.hpp"
#include "shader.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class sky : public free
    {
        float       dist;
        ogl::image  glow;
        ogl::image  fill;
        ogl::shader prog;

    public:

        sky(float);

        void draw_fill(int);
    };
}

//-----------------------------------------------------------------------------

#endif
