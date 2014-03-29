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

app::view::view() : scaling(1)
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

// Return the world-space position of a pointer given in eye coordinates.

vec3 app::view::get_point_pos(const vec3& p) const
{
    return get_inverse() * p;
}

// Return the world-space vector of a pointer given in eye coordinates.

vec3 app::view::get_point_vec(const quat& q) const
{
    return normal(transpose(get_transform()) * zvector(mat3(q)));
}

//-----------------------------------------------------------------------------

// The view matrix is the inverse of a camera's model matrix.

mat4 app::view::get_inverse() const
{
    mat4 T = translation(position);
    mat4 R = mat4(mat3(orientation));
    mat4 S = scale(vec3(1.0 / scaling, 1.0 / scaling, 1.0 / scaling));

    return T * R * S * tracking;
}

mat4 app::view::get_transform() const
{
    return inverse(get_inverse());
}

// Load the view matrix onto the OpenGL matrix stack. Convert row-major to
// column-major as required by OpenGL.

void app::view::load_transform() const
{
    glLoadMatrixd(transpose(get_transform()));
}

//-----------------------------------------------------------------------------
