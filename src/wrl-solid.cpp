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
    line_scale = fill_bound.length() / 2.0;

    if ((edit_geom = new_geom(0)))
        dGeomSetData(edit_geom, this);
}

wrl::sphere::sphere(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "wire/wire_sphere.obj")
{
    line_scale = fill_bound.length() / 2.0;

    if ((edit_geom = new_geom(0)))
        dGeomSetData(edit_geom, this);
}

wrl::convex::convex(app::node node, std::string _fill_name) :
    solid(node, _fill_name, "")
{
    data = ::glob->load_convex(line_name);
#if 0
    id = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble(id, data->get_points(), 3 * sizeof (double),
                                    data->num_points(),
                                    data->get_indices(),
                                    data->num_indices(), 3 * sizeof (unsigned int));
#endif
    if ((edit_geom = new_geom(0)))
        dGeomSetData(edit_geom, this);
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

dGeomID wrl::box::new_geom(dSpaceID space) const
{
    vec3 d = fill_bound.length();

    if (length(d) > 0)
    {
        dGeomID geom = dCreateBox(space, d[0], d[1], d[2]);
        bGeomSetTransform(geom, default_M);
        return geom;
    }
    return 0;
}

dGeomID wrl::sphere::new_geom(dSpaceID space) const
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
        return geom;
    }
    return 0;
}

dGeomID wrl::convex::new_geom(dSpaceID space) const
{
    if (data)
    {
#if 1
        dGeomID geom = dCreateConvex(space, data->get_planes(),
                                            data->num_planes(),
                                            data->get_points(),
                                            data->num_points(),
                                            data->get_polygons());
#else
        dGeomID geom = dCreateTriMesh(space, id, 0, 0, 0);
#endif
        bGeomSetTransform(geom, default_M);
        return geom;
    }
    return 0;
}

//-----------------------------------------------------------------------------

// Compute and position the mass of this box.

void wrl::box::get_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();
    dVector3 v;

    dGeomBoxGetLengths(edit_geom, v);

    dMassSetBox(mass, d, v[0], v[1], v[2]);
    printf("box mass %f %f %f -> ", mass->c[0], mass->c[1], mass->c[2]);
    bMassSetTransform(mass, current_M);
    printf("%f %f %f %s\n", mass->c[0], mass->c[1], mass->c[2], fill_name.c_str());
}

// Compute and position the mass of this sphere.

void wrl::sphere::get_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

    dMassSetSphere(mass, d, dGeomSphereGetRadius(edit_geom));
    printf("sphere mass %f %f %f -> ", mass->c[0], mass->c[1], mass->c[2]);
    bMassSetTransform(mass, current_M);
    printf("%f %f %f %s\n", mass->c[0], mass->c[1], mass->c[2], fill_name.c_str());
}

// Compute the mass of this convex using ODE's trimesh mass calculator.

void wrl::convex::get_mass(dMass *mass)
{
    dReal d = (dReal) params[param::density]->value();

#if 1
    dTriMeshDataID id = dGeomTriMeshDataCreate();
    dGeomTriMeshDataBuildDouble(id, data->get_points(), 3 * sizeof (double),
                                    data->num_points(),
                                    data->get_indices(),
                                    data->num_indices(), 3 * sizeof (unsigned int));

    dGeomID tm = dCreateTriMesh(0, id, 0, 0, 0);
    dMassSetTrimesh(mass, d, tm);
    printf("convex mass %f %f %f -> ", mass->c[0], mass->c[1], mass->c[2]);
    bMassSetTransform(mass, current_M);
    printf("%f %f %f %s\n", mass->c[0], mass->c[1], mass->c[2], fill_name.c_str());
    dGeomDestroy(tm);
    dGeomTriMeshDataDestroy(id);

#else
    dMassSetTrimesh(mass, d, play_geom);
    bMassSetTransform(mass, current_M);
#endif
}

dGeomID wrl::solid::get_geom(dSpaceID space)
{
    if ((play_geom = new_geom(space)))
        dGeomSetData(play_geom, this);

    return play_geom;
}

//-----------------------------------------------------------------------------

void wrl::solid::play_init()
{
    dBodyID body = dGeomGetBody(play_geom);

    // Orient the geom with respect to the body.

    if (body)
        bGeomSetOffsetWorld(play_geom, current_M);

    // Apply the current geom tranform to the unit.

    if (fill)
    {
        mat4 M;

        if (body)
            M = bGeomGetOffset(play_geom);
        else
            M = bGeomGetTransform(edit_geom);

        fill->transform(M, inverse(M));
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
