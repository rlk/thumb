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
#include <etc-vector.hpp>
#include <ogl-pool.hpp>
#include <wrl-solid.hpp>
#include <app-data.hpp>

//-----------------------------------------------------------------------------

wrl::solid::solid(std::string fill,
                  std::string line, bool center) : atom(fill, line, center)
{
    params[param::category] = new param("category", "4294967295");
    params[param::collide]  = new param("collide",  "4294967295");
    params[param::density]  = new param("density",  "1.0");
    params[param::mu]       = new param("mu",     "100.0");
    params[param::bounce]   = new param("bounce",   "0.5");
    params[param::soft_erp] = new param("soft_erp", "0.2");
    params[param::soft_cfm] = new param("soft_cfm", "0.0");
}

//-----------------------------------------------------------------------------

// Locate a convex hull definition for the named solid. If found, return the
// name, else throw an exception.

std::string wire_from_solid(std::string solid)
{
    std::string::size_type pos;

    if ((pos = solid.find("solid/")) != std::string::npos)
    {
        std::string name = solid.replace(pos, 5, "wire");

        if (::data->find(name))
            return name;
    }
    throw std::runtime_error("Error creating convex solid: "
                        + solid + " missing hull definition");
    return solid;
}

//-----------------------------------------------------------------------------

wrl::box::box(std::string fill, bool center)
    : solid(fill, "wire/wire_box.obj", center)
{
    edit_geom = dCreateBox(0, 1.0, 1.0, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

wrl::sphere::sphere(std::string fill, bool center)
    : solid(fill, "wire/wire_sphere.obj", center)
{
    edit_geom = dCreateSphere(0, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

wrl::convex::convex(std::string fill, bool center)
    : solid(fill, wire_from_solid(fill), center)
{
    edit_geom = dCreateSphere(0, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

//-----------------------------------------------------------------------------

void wrl::box::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        ogl::aabb bound;

        fill->merge_bound(bound);

        vec3 d = bound.max()
               - bound.min();

        line_scale = d / 2.0;

        if (d[0] > 0 &&
            d[1] > 0 &&
            d[2] > 0)
            dGeomBoxSetLengths(edit_geom, d[0], d[1], d[2]);
    }
}

void wrl::sphere::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        ogl::aabb bound;

        fill->merge_bound(bound);

        vec3 a = bound.min();
        vec3 z = bound.max();

        double r = 0;

        for (int i = 0; i < 3; ++i)
        {
            if (r < fabs(a[i])) r = fabs(a[i]);
            if (r < fabs(z[i])) r = fabs(z[i]);
        }

        line_scale = vec3(r, r, r);

        if (r > 0)
            dGeomSphereSetRadius(edit_geom, dReal(r));
    }
}

void wrl::convex::scale()
{
    // Convex solids don't scale.
}

//-----------------------------------------------------------------------------

void wrl::box::get_mass(dMass *mass)
{
    dReal    d = (dReal) params[param::density]->value();
    dVector3 v;

    // Compute and position the mass of this box.

    dGeomBoxGetLengths(edit_geom, v);
    dMassSetBox(mass, d, v[0], v[1], v[2]);

    ode_set_mass_transform(mass, current_M);
}

void wrl::sphere::get_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    // Compute and position the mass of this sphere.

    dMassSetSphere(mass, d, dGeomSphereGetRadius(edit_geom));

    ode_set_mass_transform(mass, current_M);
}

void wrl::convex::get_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    // TODO: Calculate mass using trimesh.

    dMassSetBox(mass, d, 1.0, 1.0, 1.0);

    ode_set_mass_transform(mass, current_M);
}

dGeomID wrl::solid::get_geom(dSpaceID space)
{
    return (play_geom = ode_dupe_geom(space, edit_geom));
}

//-----------------------------------------------------------------------------

void wrl::solid::play_init()
{
    dBodyID body = dGeomGetBody(play_geom);

    if (body)
    {
        // Orient the geom with respect to the body.

        mat4 I = inverse(current_M);

        dMatrix3 R;

        R[ 0] = dReal(I[0][0]);
        R[ 1] = dReal(I[1][0]);
        R[ 2] = dReal(I[2][0]);
        R[ 3] = 0;

        R[ 4] = dReal(I[0][1]);
        R[ 5] = dReal(I[1][1]);
        R[ 6] = dReal(I[2][1]);
        R[ 7] = 0;

        R[ 8] = dReal(I[0][2]);
        R[ 9] = dReal(I[1][2]);
        R[10] = dReal(I[2][2]);
        R[11] = 0;

        dGeomSetOffsetWorldRotation(play_geom, R);
        dGeomSetOffsetWorldPosition(play_geom, dReal(current_M[0][3]),
                                               dReal(current_M[1][3]),
                                               dReal(current_M[2][3]));
    }

    // Apply the current geom tranform to the unit.

    if (fill)
    {
        mat4 M;

        if (body)
            M = ode_get_geom_offset(play_geom);
        else
            M = ode_get_geom_transform(edit_geom);

        fill->transform(M, inverse(M));
    }
}

void wrl::solid::play_fini()
{
    // Reset the unit transform to the geom world position.

    if (fill)
    {
        mat4 M = ode_get_geom_transform(edit_geom);

        fill->transform(M, inverse(M));
    }
}

//-----------------------------------------------------------------------------

void wrl::solid::load(app::node node)
{
    // Load the OBJ file.

    if (app::node n = node.find("file"))
    {
        name = n.get_s();
        fill = new ogl::unit(name);
        scale();
    }
    atom::load(node);
}

void wrl::solid::save(app::node node)
{
    // Add the OBJ file reference.

    if (!name.empty())
    {
        app::node n("file");
        n.set_s(name);
        n.insert(node);
    }
    atom::save(node);
}

void wrl::box::save(app::node node)
{
    // Create a new box element.

    app::node n("geom");

    n.set_s("type", "box");
    n.insert(node);
    solid::save(n);
}

void wrl::sphere::save(app::node node)
{
    // Create a new sphere element.

    app::node n("geom");

    n.set_s("type", "sphere");
    n.insert(node);
    solid::save(n);
}

void wrl::convex::save(app::node node)
{
    // Create a new convex element.

    app::node n("geom");

    n.set_s("type", "convex");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
