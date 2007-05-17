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

static dGeomID ode_dupe_geom_sphere(dSpaceID space, dGeomID sphere)
{
    // Duplicate the given sphere geom.

    return dCreateSphere(space, dGeomSphereGetRadius(sphere));
}

static dGeomID ode_dupe_geom_box(dSpaceID space, dGeomID box)
{
    dVector3 lengths;

    dGeomBoxGetLengths(box, lengths);

    // Duplicate the given box geom.

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

    // Duplicate the geom's user data.

    dGeomSetData(next, dGeomGetData(geom));

    // Duplicate the geom's transform.

    if (next)
    {
        dQuaternion rot;

        dGeomGetQuaternion(geom, rot);
        dGeomSetQuaternion(next, rot);

        const dReal *pos = dGeomGetPosition(geom);

        dGeomSetPosition(next, pos[0], pos[1], pos[2]);
    }

    return next;
}

//-----------------------------------------------------------------------------

void ode_get_geom_transform(dGeomID geom, float M[16])
{
    const dReal *p = dGeomGetPosition(geom);
    const dReal *R = dGeomGetRotation(geom);

    M[ 0] = float(R[ 0]);
    M[ 1] = float(R[ 4]);
    M[ 2] = float(R[ 8]);
    M[ 3] = 0.0f;

    M[ 4] = float(R[ 1]);
    M[ 5] = float(R[ 5]);
    M[ 6] = float(R[ 9]);
    M[ 7] = 0.0f;

    M[ 8] = float(R[ 2]);
    M[ 9] = float(R[ 6]);
    M[10] = float(R[10]);
    M[11] = 0.0f;

    M[12] = float(p[ 0]);
    M[13] = float(p[ 1]);
    M[14] = float(p[ 2]);
    M[15] = 1.0f;
}

void ode_set_geom_transform(dGeomID geom, float M[16])
{
    dMatrix3 R;

    R[ 0] = (dReal) M[ 0];
    R[ 1] = (dReal) M[ 4];
    R[ 2] = (dReal) M[ 8];
    R[ 3] = (dReal) 0.0f;

    R[ 4] = (dReal) M[ 1];
    R[ 5] = (dReal) M[ 5];
    R[ 6] = (dReal) M[ 9];
    R[ 7] = (dReal) 0.0f;

    R[ 8] = (dReal) M[ 2];
    R[ 9] = (dReal) M[ 6];
    R[10] = (dReal) M[10];
    R[11] = (dReal) 0.0f;

    dGeomSetRotation(geom, R);
    dGeomSetPosition(geom, (dReal) M[12],
                           (dReal) M[13],
                           (dReal) M[14]);
}

void ode_set_mass_transform(dMass *mass, float M[16])
{
    dMatrix3 R;

    R[ 0] = (dReal) M[ 0];
    R[ 1] = (dReal) M[ 4];
    R[ 2] = (dReal) M[ 8];
    R[ 3] = (dReal) 0.0f;

    R[ 4] = (dReal) M[ 1];
    R[ 5] = (dReal) M[ 5];
    R[ 6] = (dReal) M[ 9];
    R[ 7] = (dReal) 0.0f;

    R[ 8] = (dReal) M[ 2];
    R[ 9] = (dReal) M[ 6];
    R[10] = (dReal) M[10];
    R[11] = (dReal) 0.0f;

    dMassRotate   (mass, R);
    dMassTranslate(mass, (dReal) M[12],
                         (dReal) M[13],
                         (dReal) M[14]);
}

//-----------------------------------------------------------------------------
