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

#include <etc-vector.hpp>
#include <ogl-opengl.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class uniform
    {
    public:

        uniform(std::string, GLsizei);
       ~uniform();

        const std::string& get_name() const { return name; }

        void set(double);
        void set(const vec2&);
        void set(const vec3&);
        void set(const vec4&);
        void set(const mat3&);
        void set(const mat4&);

        void apply(GLint) const;

    private:

        std::string name;

        GLfloat *val;
        GLsizei  len;
    };
}

//-----------------------------------------------------------------------------

#endif
