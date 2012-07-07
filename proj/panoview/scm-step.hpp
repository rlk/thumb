//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef scm_step_HPP
#define scm_step_HPP

#include <cstdio>

//------------------------------------------------------------------------------

class scm_step
{
public:

    scm_step();
    scm_step(const scm_step *,
             const scm_step *, double);
    scm_step(const scm_step *,
             const scm_step *,
             const scm_step *,
             const scm_step *, double);

    void draw() const;

    bool read (FILE *);
    bool write(FILE *);
    bool send (int);

    void transform_orientation(const double *);
    void transform_position   (const double *);
    void transform_light      (const double *);

    void set_speed  (double s) { speed   = s; }
    void set_radius (double r) { radius  = r; }
    void set_tension(double t) { tension = t; }
    void set_bias   (double b) { bias    = b; }

    void   get_matrix  (double *, double) const;
    void   get_position(double *) const;
    void   get_up      (double *) const;
    void   get_right   (double *) const;
    void   get_forward (double *) const;
    void   get_light   (double *) const;

    double get_speed()            const { return speed;   }
    double get_radius()           const { return radius;  }
    double get_tension()          const { return tension; }
    double get_bias()             const { return bias;    }

private:

    double orientation[4]; // View orientation
    double position[3];    // View point location
    double light[3];       // Light location
    double speed;          // Camera speed
    double radius;         // View point radius
    double tension;        // Hermite interpolation tension
    double bias;           // Hermite interpolation bias
};

//------------------------------------------------------------------------------

#endif
