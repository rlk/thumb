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
    tracking    = mat4();
    orientation = quat();
    position    = vec3(0.0, 1.8, 0.0);
}

//-----------------------------------------------------------------------------

vec3 app::view::get_point_pos(const vec3& p) const
{
    const mat4 M = get_transform();
    return vec3(M * vec4(p, 1));
}

vec3 app::view::get_point_vec(const quat& q) const
{
    const mat4 M = transpose(get_inverse());
    const vec3 v = -zvector(mat3(q));
    return vec3(M * vec4(v, 0));
}

//-----------------------------------------------------------------------------

mat4 app::view::get_transform() const
{
    return translation(position) * mat4(mat3(orientation)) * tracking;
}

mat4 app::view::get_inverse() const
{
    return inverse(get_transform());
}

void app::view::load_transform() const
{
    glLoadMatrixd(transpose(get_inverse()).GIMME());
}

//-----------------------------------------------------------------------------
