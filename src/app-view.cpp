//  Copyright (C) 2005-2011 Robert Kooima
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

#include <ogl-opengl.hpp>
#include <app-view.hpp>

//-----------------------------------------------------------------------------

app::view::view()
{
    go_home();
}

void app::view::go_home()
{
    head_orientation = quat();
    body_orientation = quat();
    position = vec3(0.0, 1.8, 0.0);
}

//-----------------------------------------------------------------------------

#ifdef FIXME
void app::view::get_point(vec3& P, vec3& V, const vec3& p, const quat& q) const
{
#if 0
    double M[16], v[3];

    // Determine the point direction of the given quaternion.

    quat_to_mat(M, q);

    v[0] = -M[ 8];
    v[1] = -M[ 9];
    v[2] = -M[10];

    // Transform the point position and direction to world space.

    if (P) mult_mat_vec3(P, transform_M, p);
    if (V) mult_xps_vec3(V, transform_I, v);
#endif

    vec4 v(-transpose(mat3(q))[2]);

    P = get_transform() * p;
    V = get_inverse()   * v;
}
#endif

//-----------------------------------------------------------------------------

mat4 app::view::get_transform() const
{
    return translation(position) * mat4(mat3(body_orientation))
                                 * mat4(mat4(head_orientation));
}

mat4 app::view::get_inverse() const
{
    return mat4(mat3(inverse(head_orientation)))
         * mat4(mat3(inverse(body_orientation))) * translation(-position);
}

void app::view::load_transform() const
{
    glLoadMatrixd(transpose(get_inverse()));
}

//-----------------------------------------------------------------------------
