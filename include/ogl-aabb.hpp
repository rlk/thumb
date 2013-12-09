//  Copyright (C) 2007-2011 Robert Kooima
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

#ifndef OGL_AABB_HPP
#define OGL_AABB_HPP

#include <etc-vector.hpp>
#include <ogl-range.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class aabb
    {
    public:

        aabb();
        aabb(const vec3&, const vec3&);
        aabb(const aabb&, const mat4&);

        void merge(const vec3&);
        void merge(const aabb&);

        double     get_distance(const vec3&)              const;
        ogl::range get_range   (const vec4&)              const;
        ogl::range get_range   (const vec4&, const mat4&) const;

        bool test(const vec4 *, int)                    const;
        bool test(const vec4 *, int, const mat4&, int&) const;

        void draw(bool, bool, bool, bool) const;
        void draw()                       const;

        vec3 center(double c[3]) const { return  (a + z) / 2.0; }
        vec3 offset(double c[3]) const { return -(a + z) / 2.0; }

        double xlength() const { return z[0] - a[0]; }
        double ylength() const { return z[1] - a[1]; }
        double zlength() const { return z[2] - a[2]; }

    private:

        vec3 a;
        vec3 z;

        double min(const vec4&) const;
        double max(const vec4&) const;
    };
}

//-----------------------------------------------------------------------------

#endif
