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

    dMassSetZero(&play_mass);
}

wrl::box::box(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_box.obj")
{
    line_scale = fill_bound.length() / 2.0;
    edit_geom  = new_edit_geom(0);
}

wrl::sphere::sphere(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_sphere.obj")
{
    line_scale = fill_bound.length() / 2.0;
    edit_geom  = new_edit_geom(0);
}

wrl::convex::convex(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "")
{
    data = ::glob->load_convex(line_name);

    id = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble(id, data->get_points(),
                                    data->siz_points(),
                                    data->num_points(),
                                    data->get_indices(),
                                    data->num_indices(),
                                    data->siz_indices());

    edit_geom = new_edit_geom(0);
}

//-----------------------------------------------------------------------------

// Convex solid copy constructor and destructor ensure that the convex data
// is reference counted correcly.

wrl::convex::convex(const convex& that) : solid(that)
{
    data = ::glob->dupe_convex(that.data);
}

wrl::convex::~convex()
{
    ::glob->free_convex(data);
}

//-----------------------------------------------------------------------------

dGeomID wrl::box::new_edit_geom(dSpaceID space)
{
    vec3 d = fill_bound.length();

    if (length(d) > 0)
    {
        dGeomID geom = dCreateBox(space, d[0], d[1], d[2]);
        bGeomSetTransform(geom, default_M);
        dGeomSetData(geom, this);
        return geom;
    }
    return 0;
}

dGeomID wrl::sphere::new_edit_geom(dSpaceID space)
{
    vec3 a = fill_bound.min();
    vec3 z = fill_bound.max();

    double r = 0;

    if (r < fabs(a[0])) r = fabs(a[0]);
    if (r < fabs(z[0])) r = fabs(z[0]);
    if (r < fabs(a[1])) r = fabs(a[1]);
    if (r < fabs(z[1])) r = fabs(z[1]);
    if (r < fabs(a[2])) r = fabs(a[2]);
    if (r < fabs(z[2])) r = fabs(z[2]);

    if (r > 0)
    {
        dGeomID geom = dCreateSphere(space, dReal(r));
        bGeomSetTransform(geom, default_M);
        dGeomSetData(geom, this);
        return geom;
    }
    return 0;
}

dGeomID wrl::convex::new_edit_geom(dSpaceID space)
{
    if (data)
    {
        dGeomID geom = dCreateConvex(space, data->get_planes(),
                                            data->num_planes(),
                                            data->get_points(),
                                            data->num_points(),
                                            data->get_polygons());
        bGeomSetTransform(geom, default_M);
        dGeomSetData(geom, this);
        return geom;
    }
    return 0;
}

//-----------------------------------------------------------------------------

// By default the play geom is the same as the edit geom.

dGeomID wrl::solid::new_play_geom(dSpaceID space)
{
    play_geom = new_edit_geom(space);
    return play_geom;
}

//-----------------------------------------------------------------------------

// Compute and position the mass of this box.

void wrl::box::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dVector3 v;

    dGeomBoxGetLengths(edit_geom, v);

    dMassSetBox(mass, d, v[0], v[1], v[2]);
    play_mass = *mass;
    bMassSetTransform(mass, current_M);

}

// Compute and position the mass of this sphere.

void wrl::sphere::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    dMassSetSphere(mass, d, dGeomSphereGetRadius(edit_geom));
    play_mass = *mass;
    bMassSetTransform(mass, current_M);

}

// Compute the mass of this convex using ODE's trimesh mass calculator.

void wrl::convex::new_play_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    dTriMeshDataID id = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble(id, data->get_points(),
                                    data->siz_points(),
                                    data->num_points(),
                                    data->get_indices(),
                                    data->num_indices(),
                                    data->siz_indices());

    dGeomID tm = dCreateTriMesh(0, id, 0, 0, 0);

    dMassSetTrimesh(mass, d, tm);
    play_mass = *mass;
    bMassSetTransform(mass, current_M);

    dGeomDestroy(tm);
    dGeomTriMeshDataDestroy(id);
}

//-----------------------------------------------------------------------------

void wrl::solid::play_init()
{
    dBodyID body = dGeomGetBody(play_geom);

    mat4 M = translation(vec3(double(-play_mass.c[0]),
                              double(-play_mass.c[1]),
                              double(-play_mass.c[2])));

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

void wrl::convex::save(app::node node)
{
    // Create a new convex element.

    app::node n("geom");

    n.set_s("type", "convex");
    n.insert(node);
    solid::save(n);
}

//-----------------------------------------------------------------------------
