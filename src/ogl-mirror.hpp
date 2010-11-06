//  Copyright (C) 2010 Robert Kooima
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

#ifndef OGL_MIRROR_HPP
#define OGL_MIRROR_HPP

//-----------------------------------------------------------------------------

namespace ogl
{
    class binding;
    class frame;
}

//-----------------------------------------------------------------------------

namespace ogl
{
    class mirror
    {
        const ogl::binding *binding;
        const ogl::frame   *frame;
        
    public:
        
        mirror(std::string, int, int);
       ~mirror();
       
       void bind() const;
       void free() const;
       void draw() const;
    };
}

//-----------------------------------------------------------------------------

#endif
