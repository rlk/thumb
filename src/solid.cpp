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

#include "opengl.hpp"
#include "matrix.hpp"
#include "solid.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

wrl::solid::solid(const ogl::surface *fill,
                  const ogl::surface *line) : atom(fill, line)
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

wrl::box::box(dSpaceID space, const ogl::surface *fill) :
    solid(fill, glob->load_surface("wire/wire_box.obj"))
{
    geom = dCreateBox(space, 1.0, 1.0, 1.0);

    dGeomSetData(geom, this);
    set_transform(current_M);

    scale();
}

wrl::sphere::sphere(dSpaceID space, const ogl::surface *fill) : 
    solid(fill, glob->load_surface("wire/wire_sphere.obj"))
{
    geom = dCreateSphere(space, 1.0);

    dGeomSetData(geom, this);
    set_transform(current_M);

    scale();
}

//-----------------------------------------------------------------------------

void wrl::box::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (geom && fill)
    {
        float bound[6];

        fill->box_bound(bound);

        dGeomBoxSetLengths(geom, bound[3] - bound[0],
                                 bound[4] - bound[1],
                                 bound[5] - bound[2]);
    }
}

void wrl::sphere::scale()
{
    // Apply the scale of the fill geometry to the geom.

    if (fill)
    {
        float bound[1];

        fill->sph_bound(bound);

        dGeomSphereSetRadius(geom, bound[0]);
    }
}

//-----------------------------------------------------------------------------

void wrl::box::play_init(dBodyID body)
{
    dReal    d = (dReal) params[param::density]->value();
    dVector3 v;

    // Compute the mass of this box.

    dGeomBoxGetLengths(geom, v);
//  dMassSetBox(&mass, d, v[0], v[1], v[2]);

//  solid::play_init(body);
}

void wrl::sphere::play_init(dBodyID body)
{
    dReal d = (dReal) params[param::density]->value();
    dReal r;

    // Compute the mass of this sphere.

    r = dGeomSphereGetRadius(geom);
//  dMassSetSphere(&mass, d, r);

//  solid::play_init(body);
}

//-----------------------------------------------------------------------------

void wrl::solid::step_post()
{
    // Update the current transform using the current ODE state.

    get_transform(current_M);
}

//-----------------------------------------------------------------------------

void wrl::box::draw_line() const
{
    dVector3 v;

    dGeomBoxGetLengths(geom, v);

    // Draw a wire box.

    glPushMatrix();
    {
        mult_M();

        glScalef(float(v[0] / 2.0f),
                 float(v[1] / 2.0f),
                 float(v[2] / 2.0f));

        line->draw(DRAW_OPAQUE | DRAW_UNLIT);
    }
    glPopMatrix();
}

void wrl::sphere::draw_line() const
{
    float r = float(dGeomSphereGetRadius(geom));

    // Draw a wire sphere.

    glPushMatrix();
    {
        mult_M();

        glScalef(r, r, r);

        line->draw(DRAW_OPAQUE | DRAW_UNLIT);
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

        fill = glob->load_surface(s);

        scale();
    }
    atom::load(node);
}

mxml_node_t *wrl::solid::save(mxml_node_t *node)
{
    // Add the OBJ file reference.

    if (fill)
    {
        std::string s = fill->get_name();

        mxmlNewText(mxmlNewElement(node, "file"), 0, s.c_str());
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
