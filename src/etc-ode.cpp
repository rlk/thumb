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

void ode_get_body_transform(dBodyID body, double *M)
{
    const dReal *p = dBodyGetPosition(body);
    const dReal *R = dBodyGetRotation(body);

    M[ 0] = double(R[ 0]);
    M[ 1] = double(R[ 4]);
    M[ 2] = double(R[ 8]);
    M[ 3] = 0;

    M[ 4] = double(R[ 1]);
    M[ 5] = double(R[ 5]);
    M[ 6] = double(R[ 9]);
    M[ 7] = 0;

    M[ 8] = double(R[ 2]);
    M[ 9] = double(R[ 6]);
    M[10] = double(R[10]);
    M[11] = 0;

    M[12] = double(p[ 0]);
    M[13] = double(p[ 1]);
    M[14] = double(p[ 2]);
    M[15] = 1;
}

void ode_get_geom_offset(dGeomID geom, double *M)
{
    const dReal *p = dGeomGetOffsetPosition(geom);
    const dReal *R = dGeomGetOffsetRotation(geom);

    M[ 0] = double(R[ 0]);
    M[ 1] = double(R[ 4]);
    M[ 2] = double(R[ 8]);
    M[ 3] = 0;

    M[ 4] = double(R[ 1]);
    M[ 5] = double(R[ 5]);
    M[ 6] = double(R[ 9]);
    M[ 7] = 0;

    M[ 8] = double(R[ 2]);
    M[ 9] = double(R[ 6]);
    M[10] = double(R[10]);
    M[11] = 0;

    M[12] = double(p[ 0]);
    M[13] = double(p[ 1]);
    M[14] = double(p[ 2]);
    M[15] = 1;
}

void ode_get_geom_transform(dGeomID geom, double *M)
{
    const dReal *p = dGeomGetPosition(geom);
    const dReal *R = dGeomGetRotation(geom);

    M[ 0] = double(R[ 0]);
    M[ 1] = double(R[ 4]);
    M[ 2] = double(R[ 8]);
    M[ 3] = 0;

    M[ 4] = double(R[ 1]);
    M[ 5] = double(R[ 5]);
    M[ 6] = double(R[ 9]);
    M[ 7] = 0;

    M[ 8] = double(R[ 2]);
    M[ 9] = double(R[ 6]);
    M[10] = double(R[10]);
    M[11] = 0;

    M[12] = double(p[ 0]);
    M[13] = double(p[ 1]);
    M[14] = double(p[ 2]);
    M[15] = 1;
}

void ode_set_geom_transform(dGeomID geom, double *M)
{
    dMatrix3 R;

    R[ 0] = dReal(M[ 0]);
    R[ 1] = dReal(M[ 4]);
    R[ 2] = dReal(M[ 8]);
    R[ 3] = 0;

    R[ 4] = dReal(M[ 1]);
    R[ 5] = dReal(M[ 5]);
    R[ 6] = dReal(M[ 9]);
    R[ 7] = 0;

    R[ 8] = dReal(M[ 2]);
    R[ 9] = dReal(M[ 6]);
    R[10] = dReal(M[10]);
    R[11] = 0;

    dGeomSetRotation(geom, R);
    dGeomSetPosition(geom, dReal(M[12]),
                           dReal(M[13]),
                           dReal(M[14]));
}

void ode_set_mass_transform(dMass *mass, double *M)
{
    dMatrix3 R;

    R[ 0] = dReal(M[ 0]);
    R[ 1] = dReal(M[ 4]);
    R[ 2] = dReal(M[ 8]);
    R[ 3] = 0;

    R[ 4] = dReal(M[ 1]);
    R[ 5] = dReal(M[ 5]);
    R[ 6] = dReal(M[ 9]);
    R[ 7] = 0;

    R[ 8] = dReal(M[ 2]);
    R[ 9] = dReal(M[ 6]);
    R[10] = dReal(M[10]);
    R[11] = 0;

    dMassRotate   (mass, R);
    dMassTranslate(mass, dReal(M[12]),
                         dReal(M[13]),
                         dReal(M[14]));
}

//-----------------------------------------------------------------------------
