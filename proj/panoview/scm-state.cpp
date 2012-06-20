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
#include "scm-state.hpp"

//------------------------------------------------------------------------------

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

scm_state::scm_state()
{
    orientation[0] = 0.0;
    orientation[1] = 0.0;
    orientation[2] = 0.0;
    orientation[3] = 1.0;

    position[0]    = 0.0;
    position[1]    = 0.0;
    position[2]    = 0.0;
    position[3]    = 1.0;

    light[0]       = 0.0;
    light[1]       = 0.0;
    light[2]       = 0.0;
    light[3]       = 1.0;

    radius         = 0.0;
    scale          = 1.0;
    zoom           = 1.0;
}

// Initialize a new SCM viewer state using cubic interpolation of given states.

scm_state::scm_state(const scm_state *a,
                     const scm_state *b,
                     const scm_state *c,
                     const scm_state *d, double t)
{
    qsquad(orientation, a->orientation,
                        b->orientation,
                        c->orientation,
                        d->orientation);
    qsquad(position,    a->position,
                        b->position,
                        c->position,
                        d->position);

    radius = mix(a->radius, b->radius, c->radius, d->radius);
    scale  = mix(a->scale,  b->scale,  c->scale,  d->scale);
    zoom   = mix(a->zoom,   b->zoom,   c->zoom,   d->zoom);
}

//------------------------------------------------------------------------------

void scm_state::read(FILE *stream)
{
}

void scm_state::write(FILE *stream)
{
}

//------------------------------------------------------------------------------

void scm_state::transform_orientation(const double *)
{
}

void scm_state::transform_position(const double *)
{
}

void scm_state::transform_light(const double *)
{
}

//------------------------------------------------------------------------------

void scm_state::get_matrix(double *) const
{
}

//------------------------------------------------------------------------------
