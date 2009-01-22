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

#ifndef OGL_SURFACE_HPP
#define OGL_SURFACE_HPP

#include <string>
#include <memory>

#include "obj.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class surface
    {
        std::string             name;
        std::auto_ptr<obj::obj> data;

    public:

        const std::string& get_name() const { return name; }

        surface(std::string name) : name(name), data(new obj::obj(name)) { }

        // Mesh accessors

        GLsizei     max_mesh()          const { return data->max_mesh();  }
        const mesh *get_mesh(GLsizei i) const { return data->get_mesh(i); }
    };
}

//-----------------------------------------------------------------------------

#endif
