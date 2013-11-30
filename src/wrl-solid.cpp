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

//-----------------------------------------------------------------------------

wrl::solid::solid(std::string fill,
                  std::string line) : atom(fill, line)
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

wrl::box::box(std::string fill) : solid(fill, "wire/wire_box.obj")
{
    edit_geom = dCreateBox(0, 1.0, 1.0, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

wrl::sphere::sphere(std::string fill) : solid(fill, "wire/wire_sphere.obj")
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

        line_scale[0] = bound.xlength() / 2;
        line_scale[1] = bound.ylength() / 2;
        line_scale[2] = bound.zlength() / 2;

        dGeomBoxSetLengths(edit_geom, bound.xlength(),
                                      bound.ylength(),
                                      bound.zlength());
    }
}

void wrl::sphere::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        ogl::aabb bound;

        fill->merge_bound(bound);

        line_scale[0] = bound.xlength() / 2;
        line_scale[1] = bound.xlength() / 2;
        line_scale[2] = bound.xlength() / 2;

        dGeomSphereSetRadius(edit_geom, bound.xlength() / 2);
    }
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

    n.set_s("class", "box");
    n.insert(node);
    solid::save(n);
}

void wrl::sphere::save(app::node node)
{
    // Create a new sphere element.

    app::node n("geom");

    n.set_s("class", "sphere");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
