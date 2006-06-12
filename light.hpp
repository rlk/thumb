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

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "solid.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class light : public free
    {
        GLenum name;

    public:

        light(GLenum n);

        void draw_fill() const;
    };
}

//-----------------------------------------------------------------------------

#endif
