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

#include <iostream>
#include <sstream>

//-----------------------------------------------------------------------------

// Extract a convex hull definition from the given OBJ data string.

void ode_load_convex(const char *str, std::vector<dReal>&        planes,
                                      std::vector<dReal>&        points,
                                      std::vector<unsigned int>& polygons)
{
    std::stringstream data(str);
    std::string       line;

    unsigned int i;
    dReal        x;
    dReal        y;
    dReal        z;

    // Parse the given string as an OBJ.

    while (getline(data, line))
    {
        std::stringstream cmd(line);
        std::string       key;

        cmd >> key;

        if      (key == "v")
        {
            cmd >> x >> y >> z;
            points.push_back(x);
            points.push_back(y);
            points.push_back(z);
        }
        else if (key == "l")
        {
            std::vector<unsigned int> indices;

            while (cmd >> i)
                indices.push_back(i - 1);

            polygons.insert(polygons.end(), indices.size() - 1);
            polygons.insert(polygons.end(), indices.begin(), indices.end() - 1);
        }
    }

    if (!points.empty())
    {
        // Compute the bounding volume of the point cloud.

        dReal a[3];
        dReal b[3];

        a[0] = b[0] = points[0];
        a[1] = b[1] = points[1];
        a[2] = b[2] = points[2];

        for (i = 0; i < points.size(); i += 3)
        {
            if (points[i + 0] < a[0]) a[0] = points[i + 0];
            if (points[i + 1] < a[1]) a[1] = points[i + 1];
            if (points[i + 2] < a[2]) a[2] = points[i + 2];

            if (points[i + 0] > b[0]) b[0] = points[i + 0];
            if (points[i + 1] > b[1]) b[1] = points[i + 1];
            if (points[i + 2] > b[2]) b[2] = points[i + 2];
        }

        // Center the point cloud on the origin.

        for (i = 0; i < points.size(); i+= 3)
        {
            points[i + 0] -= (b[0] + a[0]) / 2;
            points[i + 1] -= (b[1] + a[1]) / 2;
            points[i + 2] -= (b[2] + a[2]) / 2;
        }

        // Compute the plane of each polygon.

        for (i = 0; i < polygons.size();)
        {
            unsigned int n = polygons[i];
            unsigned int a = polygons[i + 1];
            unsigned int b = polygons[i + 2];
            unsigned int c = polygons[i + 3];

            dReal u[3];
            dReal v[3];
            dReal p[4];

            u[0] = points[b * 3 + 0] - points[a * 3 + 0];
            u[1] = points[b * 3 + 1] - points[a * 3 + 1];
            u[2] = points[b * 3 + 2] - points[a * 3 + 2];

            v[0] = points[c * 3 + 0] - points[a * 3 + 0];
            v[1] = points[c * 3 + 1] - points[a * 3 + 1];
            v[2] = points[c * 3 + 2] - points[a * 3 + 2];

            p[0] = u[1] * v[2] - u[2] * v[1];
            p[1] = u[2] * v[0] - u[0] * v[2];
            p[2] = u[0] * v[1] - u[1] * v[0];

            dReal d = dReal(sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]));

            p[0] /= d;
            p[1] /= d;
            p[2] /= d;
            p[3]  = p[0] * points[a * 3 + 0]
                  + p[1] * points[a * 3 + 1]
                  + p[2] * points[a * 3 + 2];

            planes.push_back(p[0]);
            planes.push_back(p[1]);
            planes.push_back(p[2]);
            planes.push_back(p[3]);

            i += n + 1;
        }
    }
}

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
