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
#include <app-glob.hpp>

//-----------------------------------------------------------------------------

wrl::solid::solid(app::node node, std::string _fill_name,
                                  std::string _line_name) :
    atom(node, _fill_name, _line_name)
{
    params[param::category] = new param("category", "4294967295");
    params[param::collide]  = new param("collide",  "4294967295");
    params[param::density]  = new param("density",  "1.0");
    params[param::mu]       = new param("mu",     "100.0");
    params[param::bounce]   = new param("bounce",   "0.5");
    params[param::soft_erp] = new param("soft_erp", "0.2");
    params[param::soft_cfm] = new param("soft_cfm", "0.0");

    load_params(node);
}

wrl::box::box(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_box.obj")
{
    length = fill->get_bound().length();

    line_scale = length / 2;

    init_edit_geom(0);
}

wrl::sphere::sphere(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_sphere.obj"), radius(0)
{
    const vec3 a = fill->get_bound().min();
    const vec3 b = fill->get_bound().max();

    radius = std::max(radius, std::max(fabs(a[0]), fabs(b[0])));
    radius = std::max(radius, std::max(fabs(a[1]), fabs(b[1])));
    radius = std::max(radius, std::max(fabs(a[2]), fabs(b[2])));

    line_scale = vec3(radius, radius, radius);

    init_edit_geom(0);
}

wrl::capsule::capsule(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_cylinder.obj")
{
    const vec3 v = fill->get_bound().length();

    radius = std::max(v[0], v[1]) / 2.0;
    length = v[2] - radius - radius;

    line_scale = vec3(radius, radius, length / 2.0);

    init_edit_geom(0);
}

wrl::cylinder::cylinder(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_cylinder.obj")
{
    const vec3 v = fill->get_bound().length();

    radius = std::max(v[0], v[1]) / 2.0;
    length = v[2];

    line_scale = vec3(radius, radius, length / 2.0);

    init_edit_geom(0);
}

wrl::convex::convex(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "")
{
    if ((data = ::glob->load_convex(line_name)))
    {
        id = dGeomTriMeshDataCreate();
        dGeomTriMeshDataBuildDouble(id, data->get_points(),
                                        data->siz_points(),
                                        data->num_points(),
                                        data->get_indices(),
                                        data->num_indices(),
                                        data->siz_indices());
    }
    init_edit_geom(0);
}

//-----------------------------------------------------------------------------

// Convex solid copy constructor and destructor ensure that the convex data
// is reference counted correcly.

wrl::convex::convex(const convex& that) : solid(that), data(0), id(0)
{
    if ((data = ::glob->dupe_convex(that.data)))
    {
        id = dGeomTriMeshDataCreate();
        dGeomTriMeshDataBuildDouble(id, data->get_points(),
                                        data->siz_points(),
                                        data->num_points(),
                                        data->get_indices(),
                                        data->num_indices(),
                                        data->siz_indices());
    }
    init_edit_geom(0);
}

wrl::convex::~convex()
{
    if (id) dGeomTriMeshDataDestroy(id);
    if (data) ::glob->free_convex(data);
}

//-----------------------------------------------------------------------------

dGeomID wrl::box::new_edit_geom(dSpaceID space) const
{
    return dCreateBox(space, dReal(length[0]),
                             dReal(length[1]),
                             dReal(length[2]));
}

dGeomID wrl::sphere::new_edit_geom(dSpaceID space) const
{
    return dCreateSphere(space, dReal(radius));
}

dGeomID wrl::capsule::new_edit_geom(dSpaceID space) const
{
    return dCreateCapsule(space, dReal(radius), dReal(length));
}

dGeomID wrl::cylinder::new_edit_geom(dSpaceID space) const
{
    return dCreateCylinder(space, dReal(radius), dReal(length));
}

dGeomID wrl::convex::new_edit_geom(dSpaceID space) const
{
    return dCreateTriMesh(space, id, 0, 0, 0);
}

//-----------------------------------------------------------------------------

// By default the play geom is the same as the edit geom.

dGeomID wrl::solid::new_play_geom(dSpaceID space) const
{
    return new_edit_geom(space);
}

// The convex type uses a convex geom play mode but a trimesh in edit mode.
// This is because trimesh has better ray collision while convex has better
// physics stability.

dGeomID wrl::convex::new_play_geom(dSpaceID space) const
{
    if (data)
        return dCreateConvex(space, data->get_planes(),
                                    data->num_planes(),
                                    data->get_points(),
                                    data->num_points(),
                                    data->get_polygons());
    else
        return 0;
}

//-----------------------------------------------------------------------------

// Compute and position the mass of this box.

void wrl::box::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dMassSetBox(mass, d, dReal(length[0]),
                         dReal(length[1]),
                         dReal(length[2]));
}

// Compute and position the mass of this sphere.

void wrl::sphere::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dMassSetSphere(mass, d, dReal(radius));
}

// Compute and position the mass of this capsule.

void wrl::capsule::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dMassSetCapsule(mass, d, 3, dReal(radius), dReal(length));
}

// Compute and position the mass of this cylinder.

void wrl::cylinder::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dMassSetCylinder(mass, d, 3, dReal(radius), dReal(length));
}

// Compute the mass of this convex using ODE's trimesh mass calculator.

void wrl::convex::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    dGeomID geom = dCreateTriMesh(0, id, 0, 0, 0);

    dMassSetTrimesh(mass, d, geom);
    dGeomDestroy(geom);
}

//-----------------------------------------------------------------------------

// Create a play-mode geom. Associate this solid and set this transform.

dGeomID wrl::solid::init_play_geom(dSpaceID space)
{
    if ((play_geom = new_play_geom(space)))
    {
        dGeomSetData     (play_geom, this);
        bGeomSetTransform(play_geom, current_M);
    }
    return play_geom;
}

// Calculate the mass of this solid and set the mass transform.

void wrl::solid::init_play_mass(dMass *mass)
{
    new_play_mass(mass);
    bMassSetTransform(mass, current_M);
}

//-----------------------------------------------------------------------------

void wrl::solid::play_init()
{
    dBodyID body = dGeomGetBody(play_geom);
    dMass   mass;

    new_play_mass(&mass);

    mat4 M = translation(vec3(double(-mass.c[0]),
                              double(-mass.c[1]),
                              double(-mass.c[2])));

    // Orient the geom with respect to the body, accounting for center of mass.

    if (body)
        bGeomSetOffsetWorld(play_geom, current_M * M);

    // Apply the current geom tranform to the unit.

    if (fill)
    {
        mat4 T;

        if (body)
            T = bGeomGetOffset   (play_geom) * inverse(M);
        else
            T = bGeomGetTransform(edit_geom) * inverse(M);

        fill->transform(T, inverse(T));
    }
}

void wrl::solid::play_fini()
{
    // Reset the unit transform to the geom world position.

    if (fill)
    {
        mat4 M = bGeomGetTransform(edit_geom);

        fill->transform(M, inverse(M));
    }
}

//-----------------------------------------------------------------------------

void wrl::solid::save(app::node node)
{
    // Add the OBJ file reference.

    if (!fill_name.empty())
    {
        app::node n("file");
        n.set_s(fill_name);
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

void wrl::capsule::save(app::node node)
{
    // Create a new capsule element.

    app::node n("geom");

    n.set_s("type", "capsule");
    n.insert(node);
    solid::save(n);
}

void wrl::cylinder::save(app::node node)
{
    // Create a new cylinder element.

    app::node n("geom");

    n.set_s("type", "cylinder");
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
