//  Copyright (C) 2005-2012 Robert Kooima
//
//  THUMB is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software
//  Foundation; either version 2 of the License, or (at your option) any later
//  version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
//  more details.

#include <cmath>

#include "math3d.h"
#include "scm-step.hpp"

//------------------------------------------------------------------------------

// http://paulbourke.net/miscellaneous/interpolation/
// Consider a Catmull-Rom spline here.

static double mix(double a, double b, double c, double d, double t)
{
    const double w = d - c - a + b;
    const double x = a - b - w;
    const double y = c - a;
    const double z = b;

    return (w * t * t * t) + (x * t * t) + (y * t) + (z);
}

//------------------------------------------------------------------------------

// Initialize a new SCM viewer state using default values.

scm_step::scm_step()
{
    orientation[0] = 0.0;
    orientation[1] = 0.0;
    orientation[2] = 0.0;
    orientation[3] = 1.0;

    position[0]    = 1.0;
    position[1]    = 0.0;
    position[2]    = 0.0;
    position[3]    = 0.0;

    light[0]       = 1.0;
    light[1]       = 0.0;
    light[2]       = 0.0;
    light[3]       = 0.0;

    radius         = 0.0;
    scale          = 1.0;
    zoom           = 1.0;
}

// Initialize a new SCM viewer state using cubic interpolation of given states.

scm_step::scm_step(const scm_step& a,
                   const scm_step& b,
                   const scm_step& c,
                   const scm_step& d, double t)
{
    qsquad(orientation, a.orientation,
                        b.orientation,
                        c.orientation,
                        d.orientation, t);
    qsquad(position,    a.position,
                        b.position,
                        c.position,
                        d.position, t);

    qnormalize(orientation, orientation);
    qnormalize(position,    position);

    radius = mix(a.radius, b.radius, c.radius, d.radius, t);
    scale  = mix(a.scale,  b.scale,  c.scale,  d.scale,  t);
    zoom   = mix(a.zoom,   b.zoom,   c.zoom,   d.zoom,   t);
}

//------------------------------------------------------------------------------

bool scm_step::read(FILE *stream)
{
    return (fread(this, sizeof (scm_step), 1, stream) == 1);
}

bool scm_step::write(FILE *stream)
{
    return (fwrite(this, sizeof (scm_step), 1, stream) == 1);
}

//------------------------------------------------------------------------------

static inline void transform_quaternion(const double *M, double *q)
{
    double A[16];
    double B[16];

    mquaternion(A, q);
    mmultiply(B, M, A);
    qmatrix(q, B);
    qnormalize(q, q);
}

void scm_step::transform_orientation(const double *M)
{
    transform_quaternion(M, orientation);
}

void scm_step::transform_position(const double *M)
{
    transform_quaternion(M, position);
}

void scm_step::transform_light(const double *M)
{
    transform_quaternion(M, light);
}

//------------------------------------------------------------------------------

// Return the view transformation matrix.

void scm_step::get_matrix(double *M) const
{
    vquaternionx(M +  0, orientation);
    vquaterniony(M +  4, orientation);
    vquaternionz(M +  8, orientation);
    vquaternionz(M + 12, position);

    M[12] *= -radius * scale;
    M[13] *= -radius * scale;
    M[14] *= -radius * scale;

    M[ 3] = 0.0;
    M[ 7] = 0.0;
    M[11] = 0.0;
    M[15] = 1.0;
}

// Return the negated Z axis of the matrix form of the position quaternion,
// thus giving the position vector.

void scm_step::get_position(double *v) const
{
    vquaternionz(v, position);
    vneg(v, v);
}

// Return the Y axis of the matrix form of the orientation quaternion, thus
// giving the view up vector.

void scm_step::get_up(double *v) const
{
    vquaterniony(v, orientation);
}

// Return the X axis of the matrix form of the orientation quaternion, thus
// giving the view right vector.

void scm_step::get_right(double *v) const
{
    vquaternionx(v, orientation);
}

// Return the negated Z axis of the matrix form of the orientation quaternion,
// thus giving the view forward vector.

void scm_step::get_forward(double *v) const
{
    vquaternionz(v, orientation);
    vneg(v, v);
}

// Return the negated Z axis of the matrix form of the light source quaternion,
// thus giving the light direction vector.

void scm_step::get_light(double *v) const
{
    vquaternionz(v, light);
    vneg(v, v);
}

//------------------------------------------------------------------------------
