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

#include <etc-ode.hpp>

//-----------------------------------------------------------------------------

static dGeomID ode_dupe_geom_sphere(dSpaceID space, dGeomID sphere)
{
    // Duplicate the given sphere geom.

    return dCreateSphere(space, dGeomSphereGetRadius(sphere));
}

static dGeomID ode_dupe_geom_box(dSpaceID space, dGeomID box)
{
    // Duplicate the given box geom.

    dVector3 lengths;

    dGeomBoxGetLengths(box, lengths);

    return dCreateBox(space, lengths[0],
                             lengths[1],
                             lengths[2]);
}

dGeomID ode_dupe_geom(dSpaceID space, dGeomID geom)
{
    dGeomID next = 0;

    // Duplicate the given geom.

    switch (dGeomGetClass(geom))
    {
    case dBoxClass:    next = ode_dupe_geom_box   (space, geom); break;
    case dSphereClass: next = ode_dupe_geom_sphere(space, geom); break;
    }

    // Duplicate the geom's transform.

    if (next)
    {
        dQuaternion rot;

        dGeomGetQuaternion(geom, rot);
        dGeomSetQuaternion(next, rot);

        const dReal *pos = dGeomGetPosition(geom);

        dGeomSetPosition(next, pos[0], pos[1], pos[2]);

        dGeomSetData(next, dGeomGetData(geom));
    }

    return next;
}

//-----------------------------------------------------------------------------

mat4 ode_get_body_transform(dBodyID body)
{
    const dReal *p = dBodyGetPosition(body);
    const dReal *R = dBodyGetRotation(body);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

mat4 ode_get_geom_offset(dGeomID geom)
{
    const dReal *p = dGeomGetOffsetPosition(geom);
    const dReal *R = dGeomGetOffsetRotation(geom);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

mat4 ode_get_geom_transform(dGeomID geom)
{
    const dReal *p = dGeomGetPosition(geom);
    const dReal *R = dGeomGetRotation(geom);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

void ode_set_geom_transform(dGeomID geom, const mat4& M)
{
    dMatrix3 R;

    R[ 0] = dReal(M[0][0]);
    R[ 1] = dReal(M[1][0]);
    R[ 2] = dReal(M[2][0]);
    R[ 3] = 0;

    R[ 4] = dReal(M[0][1]);
    R[ 5] = dReal(M[1][1]);
    R[ 6] = dReal(M[2][1]);
    R[ 7] = 0;

    R[ 8] = dReal(M[0][2]);
    R[ 9] = dReal(M[1][2]);
    R[10] = dReal(M[2][2]);
    R[11] = 0;

    dGeomSetRotation(geom, R);
    dGeomSetPosition(geom, dReal(M[0][3]),
                           dReal(M[1][3]),
                           dReal(M[2][3]));
}

void ode_set_mass_transform(dMass *mass, const mat4& M)
{
    dMatrix3 R;

    R[ 0] = dReal(M[0][0]);
    R[ 1] = dReal(M[1][0]);
    R[ 2] = dReal(M[2][0]);
    R[ 3] = 0;

    R[ 4] = dReal(M[0][1]);
    R[ 5] = dReal(M[1][1]);
    R[ 6] = dReal(M[2][1]);
    R[ 7] = 0;

    R[ 8] = dReal(M[0][2]);
    R[ 9] = dReal(M[1][2]);
    R[10] = dReal(M[2][2]);
    R[11] = 0;

    dMassRotate   (mass, R);
    dMassTranslate(mass, dReal(M[0][3]),
                         dReal(M[1][3]),
                         dReal(M[2][3]));
}

//-----------------------------------------------------------------------------
