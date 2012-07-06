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
#include <cstdlib>

#include <ogl-opengl.hpp>

#include "math3d.h"
#include "scm-step.hpp"

//------------------------------------------------------------------------------

// http://paulbourke.net/miscellaneous/interpolation/
// Consider a Catmull-Rom spline here.

static double mix(const double *a,
                  const double *b,
                  const double *c,
                  const double *d, double t)
{
    if (b && c)
    {
        double aa = a ? *a : lerp(*b, *c, -1.0);
        double bb =     *b;
        double cc =     *c;
        double dd = d ? *d : lerp(*b, *c, +2.0);

        const double w = dd - cc - aa + bb;
        const double x = aa - bb - w;
        const double y = cc - aa;
        const double z = bb;

        return (w * t * t * t) + (x * t * t) + (y * t) + (z);
    }
    else if (b) return *b;
    else if (c) return *c;

    return 0;
}

static void qerp(double *q, const double *a,
                            const double *b,
                            const double *c,
                            const double *d, double t)
{
    double A[4];
    double D[4];

    if (b && c)
    {
        if (b[0] != c[0] ||
            b[1] != c[1] ||
            b[2] != c[2] ||
            b[3] != c[3])
        {
            if (a) qcpy(A, a); else qslerp(A, b, c, -1.0);
            if (d) qcpy(D, d); else qslerp(D, b, c, +2.0);

            qsquad(q, A, b, c, D, t);
            qnormalize(q, q);
        }
        else qcpy(q, b);
    }
    else if (b) qcpy(q, b);
    else if (c) qcpy(q, c);
    else
    {
        q[0] = 0.0;
        q[1] = 0.0;
        q[2] = 0.0;
        q[3] = 1.0;
    }
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
    speed          = 1.0;
    zoom           = 1.0;
}

// Initialize a new SCM viewer state using cubic interpolation of given states.

scm_step::scm_step(const scm_step *a,
                   const scm_step *b,
                   const scm_step *c,
                   const scm_step *d, double t)
{
    qerp(orientation, a ?  a->orientation : NULL,
                      b ?  b->orientation : NULL,
                      c ?  c->orientation : NULL,
                      d ?  d->orientation : NULL, t);
    qerp(position,    a ?  a->position    : NULL,
                      b ?  b->position    : NULL,
                      c ?  c->position    : NULL,
                      d ?  d->position    : NULL, t);
    radius = mix(     a ? &a->radius      : NULL,
                      b ? &b->radius      : NULL,
                      c ? &c->radius      : NULL,
                      d ? &d->radius      : NULL, t);
    zoom   = mix(     a ? &a->zoom        : NULL,
                      b ? &b->zoom        : NULL,
                      c ? &c->zoom        : NULL,
                      d ? &d->zoom        : NULL, t);

    if (b && c) speed = lerp(b->speed, c->speed, t);
    else if (b) speed =      b->speed;
    else if (c) speed =                c->speed;
    else        speed = 1.0;
}

void scm_step::draw() const
{
    double v[3];

    get_position(v);

    v[0] *= radius;
    v[1] *= radius;
    v[2] *= radius;

    glVertex3dv(v);
}

//------------------------------------------------------------------------------

bool scm_step::write(FILE *stream)
{
    fprintf(stream, "%+12.8f %+12.8f %+12.8f %+12.8f "
                    "%+12.8f %+12.8f %+12.8f %+12.8f "
                    "%+12.8f %+12.8f %+12.8f\n",
                    orientation[0],
                    orientation[1],
                    orientation[2],
                    orientation[3],
                    position[0],
                    position[1],
                    position[2],
                    position[3],
                    radius,
                    speed,
                    zoom);
    return true;
}

bool scm_step::read(FILE *stream)
{
    return (fscanf(stream, "%lf %lf %lf %lf "
                           "%lf %lf %lf %lf "
                           "%lf %lf %lf\n",
                           orientation + 0,
                           orientation + 1,
                           orientation + 2,
                           orientation + 3,
                           position + 0,
                           position + 1,
                           position + 2,
                           position + 3,
                          &radius,
                          &speed,
                          &zoom) == 11);
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

void scm_step::get_matrix(double *M, double scale) const
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
