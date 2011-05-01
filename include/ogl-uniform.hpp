//  Copyright (C) 2009-2011 Robert Kooima
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

#ifndef OGL_UNIFORM_HPP
#define OGL_UNIFORM_HPP

#include <string>

#include <ogl-opengl.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class uniform
    {
        std::string name;

        GLfloat *val;
        GLsizei  len;

    public:

        const std::string& get_name() const { return name; }

        uniform(std::string, GLsizei);
       ~uniform();

        void set(double);
        void set(double, double);
        void set(double, double, double);
        void set(double, double, double, double);
        void set(const double *);

        void apply(GLint) const;
    };
}

//-----------------------------------------------------------------------------

#endif
