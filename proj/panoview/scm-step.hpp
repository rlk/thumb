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

#ifndef SCM_STATE_HPP
#define SCM_STATE_HPP

#include <cstdio>

//------------------------------------------------------------------------------

class scm_state
{
public:

    scm_state();
    scm_state(const scm_state *,
              const scm_state *,
              const scm_state *,
              const scm_state *, double);

    bool read (FILE *);
    bool write(FILE *);

    void transform_orientation(const double *);
    void transform_position   (const double *);
    void transform_light      (const double *);

    void set_radius(double r) { radius = r; }
    void set_scale (double s) { scale  = s; }
    void set_zoom  (double z) { zoom   = z; }

    void   get_matrix  (double *) const;
    void   get_position(double *) const;
    void   get_up      (double *) const;
    void   get_right   (double *) const;
    void   get_forward (double *) const;
    void   get_light   (double *) const;
    double get_radius()           const { return radius; }
    double get_scale()            const { return scale;  }
    double get_zoom()             const { return zoom;   }

private:

    double orientation[4]; // Viewer orientation (quaternion)
    double position[4];    // Viewer direction   (quaternion)
    double light[4];       // Light  direction   (quaternion)
    double radius;         // Viewer radius
    double scale;          // Sphere scale factor
    double zoom;           // Sphere zoom factor
};

//------------------------------------------------------------------------------

#endif
