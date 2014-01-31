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

// This module provides utility functions that sit atop the ODE API. They
// conform to the style of ODE, with the leading d replaced by b (for Bob).

//-----------------------------------------------------------------------------

mat4 bBodyGetTransform(dBodyID body)
{
    const dReal *p = dBodyGetPosition(body);
    const dReal *R = dBodyGetRotation(body);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

mat4 bGeomGetOffset(dGeomID geom)
{
    const dReal *p = dGeomGetOffsetPosition(geom);
    const dReal *R = dGeomGetOffsetRotation(geom);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

mat4 bGeomGetTransform(dGeomID geom)
{
    const dReal *p = dGeomGetPosition(geom);
    const dReal *R = dGeomGetRotation(geom);

    return mat4(double(R[0]), double(R[1]), double(R[ 2]), double(p[0]),
                double(R[4]), double(R[5]), double(R[ 6]), double(p[1]),
                double(R[8]), double(R[9]), double(R[10]), double(p[2]));
}

void bGeomSetTransform(dGeomID geom, const mat4& M)
{
    dMatrix3 R;

    R[ 0] = dReal(M[0][0]);
    R[ 1] = dReal(M[0][1]);
    R[ 2] = dReal(M[0][2]);
    R[ 3] = 0;

    R[ 4] = dReal(M[1][0]);
    R[ 5] = dReal(M[1][1]);
    R[ 6] = dReal(M[1][2]);
    R[ 7] = 0;

    R[ 8] = dReal(M[2][0]);
    R[ 9] = dReal(M[2][1]);
    R[10] = dReal(M[2][2]);
    R[11] = 0;

    dGeomSetRotation(geom, R);
    dGeomSetPosition(geom, dReal(M[0][3]),
                           dReal(M[1][3]),
                           dReal(M[2][3]));
}

void bMassSetTransform(dMass *mass, const mat4& M)
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
