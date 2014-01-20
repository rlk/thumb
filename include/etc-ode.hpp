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

#include <ode/ode.h>
#include <etc-vector.hpp>

//-----------------------------------------------------------------------------

dGeomID ode_dupe_geom(dSpaceID, dGeomID);

mat4 ode_get_body_transform(dBodyID);
mat4 ode_get_geom_offset   (dGeomID);
mat4 ode_get_geom_transform(dGeomID);
void ode_set_geom_transform(dGeomID, const mat4&);
void ode_set_mass_transform(dMass *, const mat4&);

//-----------------------------------------------------------------------------
