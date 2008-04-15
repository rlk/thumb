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

#ifndef BINDING_HPP
#define BINDING_HPP

#include <string>

#include "ogl-program.hpp"
#include "ogl-texture.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class binding
    {
        static GLfloat split[4];

        std::string name;

        const ogl::program *depth;
        const ogl::program *color;

        const ogl::texture *diff;
        const ogl::texture *bump;

    public:

        const std::string& get_name() const { return name; }

        binding(std::string);
       ~binding();

        static void set_split(GLfloat, GLfloat, GLfloat, GLfloat);

        bool opaque() const { return true; }

        bool depth_eq(const binding *) const;
        bool color_eq(const binding *) const;

        void bind(bool) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
