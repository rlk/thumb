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

#include <vector>

#include <ode/ode.h>
#include <etc-vector.hpp>

//-----------------------------------------------------------------------------

bool bGeomIsPlaceable   (dGeomID);
mat4 bBodyGetTransform  (dBodyID);
mat4 bGeomGetOffset     (dGeomID);
mat4 bGeomGetTransform  (dGeomID);
void bGeomSetOffsetWorld(dGeomID, const mat4&);
void bGeomSetTransform  (dGeomID, const mat4&);
void bMassSetTransform  (dMass *, const mat4&);

//-----------------------------------------------------------------------------
