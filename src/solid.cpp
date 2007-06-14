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

#include <iostream>
#include <ode/ode.h>

#include "odeutil.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "solid.hpp"

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

        line_scale[0] = bound.length(0) / 2;
        line_scale[1] = bound.length(1) / 2;
        line_scale[2] = bound.length(2) / 2;

        dGeomBoxSetLengths(edit_geom, bound.length(0),
                                      bound.length(1),
                                      bound.length(2));
    }
}

void wrl::sphere::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        ogl::aabb bound;

        fill->merge_bound(bound);

        line_scale[0] = bound.radius();
        line_scale[1] = bound.radius();
        line_scale[2] = bound.radius();

        dGeomSphereSetRadius(edit_geom, bound.radius());
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
    dReal r;

    // Compute and position the mass of this sphere.

    r = dGeomSphereGetRadius(edit_geom);
    dMassSetSphere(mass, d, r);

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

    float M[16];
    float I[16];

    if (body)
    {
        // Orient the geom with respect to the body.

        load_inv(I, current_M);

        dGeomSetOffsetWorldRotation(play_geom, I);
        dGeomSetOffsetWorldPosition(play_geom, current_M[12],
                                               current_M[13],
                                               current_M[14]);
    }

    // Apply the current geom tranform to the unit.

    if (body)
        ode_get_geom_offset(play_geom, M);
    else
        ode_get_geom_transform(edit_geom, M);

    load_inv(I, M);

    if (fill) fill->transform(M, I);
}

void wrl::solid::play_fini()
{
    float M[16];
    float I[16];

    // Reset the unit transform to the geom world position.

    ode_get_geom_transform(edit_geom, M);

    load_inv(I, M);

    if (fill) fill->transform(M, I);
}

//-----------------------------------------------------------------------------

void wrl::solid::load(mxml_node_t *node)
{
    mxml_node_t *tag;

    // Load the OBJ file.

    if ((tag = mxmlFindElement(node, node, "file", 0, 0, MXML_DESCEND)))
    {
        name = std::string(tag->child->value.text.string);
        fill = new ogl::unit(name);

        scale();
    }
    atom::load(node);
}

mxml_node_t *wrl::solid::save(mxml_node_t *node)
{
    // Add the OBJ file reference.

    if (name.size())
        mxmlNewText(mxmlNewElement(node, "file"), 0, name.c_str());

    return atom::save(node);
}

mxml_node_t *wrl::box::save(mxml_node_t *parent)
{
    // Create a new box element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "box");
    return solid::save(node);
}

mxml_node_t *wrl::sphere::save(mxml_node_t *parent)
{
    // Create a new sphere element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "sphere");
    return solid::save(node);
}

//-----------------------------------------------------------------------------
