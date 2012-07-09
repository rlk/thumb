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
#include <cassert>
#include <cstdlib>

#include <ogl-opengl.hpp>

#include "math3d.h"
#include "scm-step.hpp"

//------------------------------------------------------------------------------

// http://paulbourke.net/miscellaneous/interpolation/

double hint(double y0, double y1,
            double y2, double y3,
            double t,
            double tension,
            double bias)
{
    double m0 = (y1 - y0) * (1.0 + bias) * (1.0 - tension) / 2.0
              + (y2 - y1) * (1.0 - bias) * (1.0 - tension) / 2.0;
    double m1 = (y2 - y1) * (1.0 + bias) * (1.0 - tension) / 2.0
              + (y3 - y2) * (1.0 - bias) * (1.0 - tension) / 2.0;

    double t2 = t * t;
    double t3 = t * t2;

    double a0 =  2.0 * t3 - 3.0 * t2 + 1.0;
    double a1 =        t3 - 2.0 * t2 + t;
    double a2 =        t3 -       t2;
    double a3 = -2.0 * t3 + 3.0 * t2;

    return a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2;
}

//------------------------------------------------------------------------------

// Initialize a new SCM viewer state using default values.

scm_step::scm_step()
{
    orientation[0] = 0.0;
    orientation[1] = 0.0;
    orientation[2] = 0.0;
    orientation[3] = 1.0;

    position[0]    = 0.0;
    position[1]    = 0.0;
    position[2]    = 1.0;

    light[0]       = 1.0;
    light[1]       = 0.0;
    light[2]       = 0.0;

    speed          = 1.0;
    radius         = 0.0;
    tension        = 0.0;
    bias           = 0.0;
}

// Initialize a new SCM viewer state using the given XML node.

scm_step::scm_step(app::node n)
{
    orientation[0] = n.get_f("q0", 0.0);
    orientation[1] = n.get_f("q1", 0.0);
    orientation[2] = n.get_f("q2", 0.0);
    orientation[3] = n.get_f("q3", 1.0);

    position[0]    = n.get_f("p0", 0.0);
    position[1]    = n.get_f("p1", 0.0);
    position[2]    = n.get_f("p2", 0.0);

    light[0]       = n.get_f("l0", 1.0);
    light[1]       = n.get_f("l1", 0.0);
    light[2]       = n.get_f("l2", 0.0);

    speed          = n.get_f("s",  1.0);
    radius         = n.get_f("r",  0.0);
    tension        = n.get_f("t",  0.0);
    bias           = n.get_f("b",  0.0);

    name           = n.get_s("name");
}

// Initialize a new SCM viewer step using linear interpolation of given steps.

scm_step::scm_step(const scm_step *a,
                   const scm_step *b, double t)
{
    assert(a);
    assert(b);

    double B[4];

    qsign(B, a->orientation, b->orientation);

    orientation[0] = lerp(a->orientation[0], B[0], t);
    orientation[1] = lerp(a->orientation[1], B[1], t);
    orientation[2] = lerp(a->orientation[2], B[2], t);
    orientation[3] = lerp(a->orientation[3], B[3], t);

    position[0]    = lerp(a->position[0],    b->position[0],    t);
    position[1]    = lerp(a->position[1],    b->position[1],    t);
    position[2]    = lerp(a->position[2],    b->position[2],    t);

    light[0]       = lerp(a->light[0],       b->light[0],       t);
    light[1]       = lerp(a->light[1],       b->light[1],       t);
    light[2]       = lerp(a->light[2],       b->light[2],       t);

    speed          = lerp(a->speed,          b->speed,          t);
    radius         = lerp(a->radius,         b->radius,         t);
    tension        = lerp(a->tension,        b->tension,        t);
    bias           = lerp(a->bias,           b->bias,           t);

    qnormalize(orientation, orientation);
    vnormalize(position,    position);
    vnormalize(light,       light);
}

// Initialize a new SCM viewer step using cubic interpolation of given steps.

scm_step::scm_step(const scm_step *a,
                   const scm_step *b,
                   const scm_step *c,
                   const scm_step *d, double t)
{
    assert(a);
    assert(b);
    assert(c);
    assert(d);

    double B[4];
    double C[4];
    double D[4];

    qsign(B, a->orientation, b->orientation);
    qsign(C, b->orientation, c->orientation);
    qsign(D, c->orientation, d->orientation);

    orientation[0] = hint(a->orientation[0], B[0], C[0], D[0], t,
                          b->tension, b->bias);
    orientation[1] = hint(a->orientation[1], B[1], C[1], D[1], t,
                          b->tension, b->bias);
    orientation[2] = hint(a->orientation[2], B[2], C[2], D[2], t,
                          b->tension, b->bias);
    orientation[3] = hint(a->orientation[3], B[3], C[3], D[3], t,
                          b->tension, b->bias);

    position[0]    = hint(a->position[0],
                          b->position[0],
                          c->position[0],
                          d->position[0], t, b->tension, b->bias);
    position[1]    = hint(a->position[1],
                          b->position[1],
                          c->position[1],
                          d->position[1], t, b->tension, b->bias);
    position[2]    = hint(a->position[2],
                          b->position[2],
                          c->position[2],
                          d->position[2], t, b->tension, b->bias);

    light[0]       = hint(a->light[0],
                          b->light[0],
                          c->light[0],
                          d->light[0], t, b->tension, b->bias);
    light[1]       = hint(a->light[1],
                          b->light[1],
                          c->light[1],
                          d->light[1], t, b->tension, b->bias);
    light[2]       = hint(a->light[2],
                          b->light[2],
                          c->light[2],
                          d->light[2], t, b->tension, b->bias);

    radius         = hint(a->radius,
                          b->radius,
                          c->radius,
                          d->radius,  t, b->tension, b->bias);

    speed          = lerp(b->speed,   c->speed,   t);
    tension        = lerp(b->tension, c->tension, t);
    bias           = lerp(b->bias,    c->bias,    t);

    qnormalize(orientation, orientation);
    vnormalize(position,    position);
    vnormalize(light,       light);
}

//------------------------------------------------------------------------------

void scm_step::draw() const
{
    double v[3];

    get_position(v);

    v[0] *= radius;
    v[1] *= radius;
    v[2] *= radius;

    glVertex3dv(v);
}

// Serialize this step to a new XML step element. Add attributes for only those
// properties with non-default values.

app::node scm_step::serialize() const
{
    app::node n("step");

    if (orientation[0] != 0.0) n.set_f("q0", orientation[0]);
    if (orientation[1] != 0.0) n.set_f("q1", orientation[1]);
    if (orientation[2] != 0.0) n.set_f("q2", orientation[2]);
    if (orientation[3] != 0.0) n.set_f("q3", orientation[3]);

    if (position[0]    != 0.0) n.set_f("p0", position[0]);
    if (position[1]    != 0.0) n.set_f("p1", position[1]);
    if (position[2]    != 0.0) n.set_f("p2", position[2]);

    if (light[0]       != 0.0) n.set_f("l0", light[0]);
    if (light[1]       != 0.0) n.set_f("l1", light[1]);
    if (light[2]       != 0.0) n.set_f("l2", light[2]);

    if (speed          != 1.0) n.set_f("s",  speed);
    if (radius         != 0.0) n.set_f("r",  radius);
    if (tension        != 0.0) n.set_f("t",  tension);
    if (bias           != 0.0) n.set_f("b",  bias);

    if (!name.empty()) n.set_s("name", name);

    return n;
}

//------------------------------------------------------------------------------

#if 0
bool scm_step::write(FILE *stream)
{
    fprintf(stream, "%+12.8f %+12.8f %+12.8f %+12.8f "
                    "%+12.8f %+12.8f %+12.8f "
                    "%+12.8f %+12.8f %+12.8f "
                    "%+12.8f %+12.8f %+12.8f %+12.8f\n",
                    orientation[0],
                    orientation[1],
                    orientation[2],
                    orientation[3],
                    position[0],
                    position[1],
                    position[2],
                    light[0],
                    light[1],
                    light[2],
                    speed,
                    radius,
                    tension,
                    bias);
    return true;
}

static bool scm_step::read(FILE *stream)
{
    return (fscanf(stream, "%lf %lf %lf %lf "
                           "%lf %lf %lf "
                           "%lf %lf %lf "
                           "%lf %lf %lf %lf\n",
                           orientation + 0,
                           orientation + 1,
                           orientation + 2,
                           orientation + 3,
                           position + 0,
                           position + 1,
                           position + 2,
                           light + 0,
                           light + 1,
                           light + 2,
                          &speed,
                          &radius,
                          &tension,
                          &bias) == 14);
}
#endif
//------------------------------------------------------------------------------

void scm_step::transform_orientation(const double *M)
{
    double A[16];
    double B[16];

    mquaternion(A, orientation);
    mmultiply(B, M, A);
    qmatrix(orientation, B);
    qnormalize(orientation, orientation);
}

void scm_step::transform_position(const double *M)
{
    double v[3];

    vtransform(v, M, position);
    vnormalize(position, v);
}

void scm_step::transform_light(const double *M)
{
    double v[3];

    vtransform(v, M, light);
    vnormalize(light, v);
}

//------------------------------------------------------------------------------

// Return the view transformation matrix.

void scm_step::get_matrix(double *M, double scale) const
{
    vquaternionx(M +  0, orientation);
    vquaterniony(M +  4, orientation);
    vquaternionz(M +  8, orientation);

    vcpy(M + 12, position);

    M[12] *= radius * scale;
    M[13] *= radius * scale;
    M[14] *= radius * scale;

    M[ 3] = 0.0;
    M[ 7] = 0.0;
    M[11] = 0.0;
    M[15] = 1.0;
}

// Return the position vector.

void scm_step::get_position(double *v) const
{
    vcpy(v, position);
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

// Return the light direction vector.

void scm_step::get_light(double *v) const
{
    vcpy(v, light);
}

//------------------------------------------------------------------------------
