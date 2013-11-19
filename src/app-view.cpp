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

#include <app-default.hpp>
#include <ogl-opengl.hpp>
#include <etc-math.hpp>
#include <app-view.hpp>
#include <app-data.hpp>
#include <app-conf.hpp>

//-----------------------------------------------------------------------------

app::view::view()
{
    go_home();
}

void app::view::go_home()
{
    load_idt(look_M);
    load_idt(look_I);

    load_xlt_mat(move_M, 0.0, 1.8, 0.0);
    load_xlt_inv(move_I, 0.0, 1.8, 0.0);

    mult_mat_mat(transform_M, move_M, look_M);
    mult_mat_mat(transform_I, look_I, move_I);
}

//-----------------------------------------------------------------------------

void app::view::get_point(double *P, const double *p,
                          double *V, const double *q) const
{
    double M[16], v[3];

    // Determine the point direction of the given quaternion.

    quat_to_mat(M, q);

    v[0] = -M[ 8];
    v[1] = -M[ 9];
    v[2] = -M[10];

    // Transform the point position and direction to world space.

    if (P) mult_mat_vec3(P, transform_M, p);
    if (V) mult_xps_vec3(V, transform_I, v);
}

//-----------------------------------------------------------------------------

void app::view::set_move_matrix(const double *M)
{
    load_mat(move_M, M);
    load_inv(move_I, M);

    mult_mat_mat(transform_M, move_M, look_M);
    mult_mat_mat(transform_I, look_I, move_I);
}

void app::view::set_look_matrix(const double *M)
{
    load_mat(look_M, M);
    load_inv(look_I, M);

    mult_mat_mat(transform_M, move_M, look_M);
    mult_mat_mat(transform_I, look_I, move_I);
}

// Load the current transformation onto the OpenGL matrix stack. This is a view
// matrix rather than a model matrix, so use the inverse.

void app::view::load_transform() const
{
    glLoadMatrixd(transform_I);
}

//-----------------------------------------------------------------------------
