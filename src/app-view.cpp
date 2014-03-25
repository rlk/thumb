//  Copyright (C) 2005-2013 Robert Kooima
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
#include <app-conf.hpp>

//-----------------------------------------------------------------------------

app::view::view() :
    scaling(1),
    vertical(bool(::conf->get_i("view_lock_vertical", 0)))
{
    go_home();
}

void app::view::go_home()
{
    tracking    = mat4();
    orientation = quat();
    position    = vec3(0.0, 0.0, 0.0);
}

//-----------------------------------------------------------------------------

vec3 app::view::get_point_pos(const vec3& p) const
{
    return get_inverse() * p;
}

vec3 app::view::get_point_vec(const quat& q) const
{
    return normal(transpose(get_transform()) * zvector(mat3(q)));
}

//-----------------------------------------------------------------------------

void app::view::set_orientation(const quat& q)
{
    if (vertical)
    {
        vec3 x(xvector(mat3(q)));
        vec3 y(0, 1, 0);
        vec3 z(0, 0, 1);

        z = normal(cross(x, y));
        x = normal(cross(y, z));

        orientation = quat(mat3(x, y, z));
    }
    else orientation = q;
}

//-----------------------------------------------------------------------------

mat4 app::view::get_inverse() const
{
    const double s = 1 / scaling;
    return translation(position)
         * mat4(mat3(orientation))
         * scale(vec3(s, s, s))
         * tracking;
}

mat4 app::view::get_transform() const
{
    return inverse(get_inverse());
}

void app::view::load_transform() const
{
    glLoadMatrixd(transpose(get_transform()));
}

//-----------------------------------------------------------------------------
