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

        bool isvalid() const;

        void merge(const vec3&);
        void merge(const aabb&);

        void intersect(const aabb&);

        bool test(const vec4 *, int)                    const;
        bool test(const vec4 *, int, const mat4&, int&) const;

        void draw(bool, bool, bool, bool) const;
        void draw()                       const;

        vec3 center() const { return  (a + z) / 2.0; }
        vec3 offset() const { return -(a + z) / 2.0; }

        vec3    min() const { return a; }
        vec3    max() const { return z; }

        double  min(const vec4&) const;
        double  max(const vec4&) const;

    private:

        vec3 a;
        vec3 z;
    };
}

//-----------------------------------------------------------------------------

#endif
