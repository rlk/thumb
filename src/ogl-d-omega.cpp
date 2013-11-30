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

#include <cassert>

#include <etc-vector.hpp>
#include <app-conf.hpp>
#include <ogl-d-omega.hpp>

//-----------------------------------------------------------------------------

ogl::d_omega::d_omega(const std::string& name) :
    cubelut(name, ::conf->get_i("reflection_cubemap_size", 128))
{
    init();
}

ogl::d_omega::~d_omega()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::d_omega::fill(float *p, const vec3& a,
                                  const vec3& b,
                                  const vec3& c,
                                  const vec3& d) const
{
    vec3 u = b - a;
    vec3 v = d - a;

    int i;
    int j;
    int k;

    for (k = 0, i = -1; i <= n; ++i)
        for (j = -1; j <= n; ++j, ++k)
        {
            const double s0 = (double(j) + 0.0) / double(n);
            const double s1 = (double(j) + 1.0) / double(n);
            const double t0 = (double(i) + 0.0) / double(n);
            const double t1 = (double(i) + 1.0) / double(n);

            vec3 A = normal(vec3(a[0] + u[0] * s0 + v[0] * t0,
                                 a[1] + u[1] * s0 + v[1] * t0,
                                 a[2] + u[2] * s0 + v[2] * t0));

            vec3 B = normal(vec3(a[0] + u[0] * s1 + v[0] * t0,
                                 a[1] + u[1] * s1 + v[1] * t0,
                                 a[2] + u[2] * s1 + v[2] * t0));

            vec3 C = normal(vec3(a[0] + u[0] * s1 + v[0] * t1,
                                 a[1] + u[1] * s1 + v[1] * t1,
                                 a[2] + u[2] * s1 + v[2] * t1));

            vec3 D = normal(vec3(a[0] + u[0] * s0 + v[0] * t1,
                                 a[1] + u[1] * s0 + v[1] * t1,
                                 a[2] + u[2] * s0 + v[2] * t1));

            p[k] = float(angle(D, C, B, A) / (4.0 * PI));
        }
}

//-----------------------------------------------------------------------------
