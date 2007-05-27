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

wrl::box::box(dSpaceID space, std::string fill) :
    solid(fill, "wire/wire_box.obj")
{
    edit_geom = dCreateBox(space, 1.0, 1.0, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

wrl::sphere::sphere(dSpaceID space, std::string fill) : 
    solid(fill, "wire/wire_sphere.obj")
{
    edit_geom = dCreateSphere(space, 1.0);

    dGeomSetData(edit_geom, this);

    scale();
}

//-----------------------------------------------------------------------------

void wrl::box::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        float bound[6];

        fill->box_bound(bound);

        dGeomBoxSetLengths(edit_geom, bound[3] - bound[0],
                                      bound[4] - bound[1],
                                      bound[5] - bound[2]);
    }
}

void wrl::sphere::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (edit_geom && fill)
    {
        float bound[1];

        fill->sph_bound(bound);

        dGeomSphereSetRadius(edit_geom, bound[0]);
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
    if (dBodyID body = dGeomGetBody(play_geom))
    {
        float I[16];

        // Orient the geom with respect to the body.

        load_inv(I, current_M);

        dGeomSetOffsetWorldRotation(play_geom, I);
        dGeomSetOffsetWorldPosition(play_geom, current_M[12],
                                               current_M[13],
                                               current_M[14]);

        // Ensure the geom's real position is correctly initialized.

        const dReal *p = dBodyGetPosition(body);

        dBodySetPosition(body, p[0], p[1], p[2]);
    }
}

void wrl::solid::step_fini()
{
    // Update the current transform using the current ODE state.

    ode_get_geom_transform(play_geom, current_M);
}

//-----------------------------------------------------------------------------

void wrl::box::draw_line() const
{
    dVector3 v;

    dGeomBoxGetLengths(edit_geom, v);

    // Draw a wire box.

    glPushMatrix();
    {
        mult_M();

        glScalef(float(v[0] / 2.0f),
                 float(v[1] / 2.0f),
                 float(v[2] / 2.0f));

//      line->draw(DRAW_OPAQUE | DRAW_UNLIT);
    }
    glPopMatrix();
}

void wrl::sphere::draw_line() const
{
    float r = float(dGeomSphereGetRadius(edit_geom));

    // Draw a wire sphere.

    glPushMatrix();
    {
        mult_M();

        glScalef(r, r, r);

//      line->draw(DRAW_OPAQUE | DRAW_UNLIT);
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

void wrl::solid::load(mxml_node_t *node)
{
    mxml_node_t *name;

    // Load the OBJ file.

    if ((name = mxmlFindElement(node, node, "file", 0, 0, MXML_DESCEND)))
    {
        std::string s = std::string(name->child->value.text.string);

        fill = new ogl::element(s);

        scale();
    }
    atom::load(node);
}

mxml_node_t *wrl::solid::save(mxml_node_t *node)
{
    // Add the OBJ file reference.

    if (name.size())
    {
        mxmlNewText(mxmlNewElement(node, "file"), 0, name.c_str());
    }
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
