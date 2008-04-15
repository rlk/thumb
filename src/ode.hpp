//  Copyright (C) 2005 Robert Kooima
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

#include <ode/ode.h>

//-----------------------------------------------------------------------------

dGeomID ode_dupe_geom(dSpaceID, dGeomID);

void ode_get_body_transform(dBodyID, double *);
void ode_get_geom_offset   (dGeomID, double *);
void ode_get_geom_transform(dGeomID, double *);
void ode_set_geom_transform(dGeomID, double *);
void ode_set_mass_transform(dMass *, double *);

//-----------------------------------------------------------------------------
