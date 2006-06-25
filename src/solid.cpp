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
#include "main.hpp"

//-----------------------------------------------------------------------------

ent::solid::solid(int f) : entity(f), tran(0)
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

void ent::box::edit_init()
{
    float bound[6];

    if (file >= 0)
    {
        obj_get_file_aabb(file, bound);

        geom = dCreateBox(space, bound[3] - bound[0],
                                 bound[4] - bound[1],
                                 bound[5] - bound[2]);
        dGeomSetData(geom, this);

        set_transform(current_M, geom);
    }
}

void ent::sphere::edit_init()
{
    if (file >= 0)
    {
        float radius = obj_get_file_sphr(file);

        geom = dCreateSphere(space, radius);

        dGeomSetData(geom, this);

        set_transform(current_M, geom);
    }
}

void ent::capsule::edit_init()
{
    float bound[6];

    if (file >= 0)
    {
        obj_get_file_aabb(file, bound);

        float length = (bound[5] - bound[2]);
        float radius = (bound[3] - bound[0]) / 2.0f;

        geom = dCreateCCylinder(space, radius, length - radius - radius);

        dGeomSetData(geom, this);

        set_transform(current_M, geom);
    }
}

void ent::plane::edit_init()
{
    geom = dCreatePlane(space, 0.0f, 1.0f, 0.0f, 0.0f);

    dGeomSetData        (geom, this);
    dGeomSetCategoryBits(geom, 0);
}

//-----------------------------------------------------------------------------

void ent::solid::play_init(dBodyID body)
{
    if (body)
    {
        dMass total;

        // Transform the mass to align with the entity.

        dMatrix3 R;

        R[ 0] = (dReal) current_M[ 0];
        R[ 1] = (dReal) current_M[ 4];
        R[ 2] = (dReal) current_M[ 8];
        R[ 3] = (dReal) 0.0f;

        R[ 4] = (dReal) current_M[ 1];
        R[ 5] = (dReal) current_M[ 5];
        R[ 6] = (dReal) current_M[ 9];
        R[ 7] = (dReal) 0.0f;

        R[ 8] = (dReal) current_M[ 2];
        R[ 9] = (dReal) current_M[ 6];
        R[10] = (dReal) current_M[10];
        R[11] = (dReal) 0.0f;
    
        dMassRotate   (&mass, R);
        dMassTranslate(&mass, current_M[12],
                              current_M[13],
                              current_M[14]);

        // Accumulate the geom's mass with the body's existing mass.

        dBodyGetMass(body, &total);
        dMassAdd(&total, &mass);
        dBodySetMass(body, &total);
    }
}

void ent::solid::play_tran(dBodyID body)
{
    if (body)
    {
        const dReal *p = dBodyGetPosition(body);

        // Create a transform geom and reposition the enclosed geom.

        tran = dCreateGeomTransform(space);
        dGeomTransformSetGeom(tran, geom);
        dGeomTransformSetInfo(tran, 0);

        dGeomSetBody(tran, body);
        dGeomSetPosition(geom, current_M[12] - p[0],
                               current_M[13] - p[1],
                               current_M[14] - p[2]);

        dSpaceRemove(space, geom);

        // Apply category and collide bits.

        unsigned int cat = (unsigned int) params[param::category]->value();
        unsigned int col = (unsigned int) params[param::collide ]->value();

        dGeomSetCategoryBits(tran, cat);
        dGeomSetCollideBits (tran, col);
    }
}

void ent::solid::play_fini()
{
    if (tran)
    {
        // Delete the transform geom and revert the transform.

        dSpaceAdd(space, geom);
        dGeomDestroy(tran);
        get_default();

        tran = 0;
    }
}

//-----------------------------------------------------------------------------

void ent::box::play_init(dBodyID body)
{
    if (body)
    {
        dVector3 v;
        dReal    d = (dReal) params[param::density]->value();

        // Compute the mass of this box.

        dGeomBoxGetLengths(geom, v);
        dMassSetBox(&mass, d, v[0], v[1], v[2]);

        solid::play_init(body);
    }
}

void ent::sphere::play_init(dBodyID body)
{
    if (body)
    {
        dReal r;
        dReal d = (dReal) params[param::density]->value();

        // Compute the mass of this sphere.

        r = dGeomSphereGetRadius(geom);
        dMassSetSphere(&mass, d, r);

        solid::play_init(body);
    }
}

void ent::capsule::play_init(dBodyID body)
{
    if (body)
    {
        dReal r;
        dReal l;
        dReal d = (dReal) params[param::density]->value();

        // Compute the mass of this sphere.

        dGeomCCylinderGetParams(geom, &r, &l);
        dMassSetCappedCylinder(&mass, d, 3, r, l);

        solid::play_init(body);
    }
}

//-----------------------------------------------------------------------------

void ent::solid::step_post()
{
    // Update the transform using the current ODE state.

    if (geom && tran)
    {
        float tran_M[16];
        float geom_M[16];

        get_transform(tran_M, tran);
        get_transform(geom_M, geom);

        mult_mat_mat(current_M, tran_M, geom_M);
    }
}

//-----------------------------------------------------------------------------

void ent::box::draw_geom() const
{
    if (geom)
    {
        dVector3 len;

        dGeomBoxGetLengths(geom, len);

        // Draw a wire box.

        glScalef(float(len[0] / 2.0f),
                 float(len[1] / 2.0f),
                 float(len[2] / 2.0f));

        ogl::draw_cube();
    }
}

void ent::sphere::draw_geom() const
{
    if (geom)
    {
        float rad = float(dGeomSphereGetRadius(geom));

        // Draw three rings.

        glScalef(rad, rad, rad);

        ogl::draw_disc(1);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        ogl::draw_disc(1);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        ogl::draw_disc(1);
    }
}

void ent::capsule::draw_geom() const
{
}

//-----------------------------------------------------------------------------

void ent::solid::load(mxml_node_t *node)
{
    mxml_node_t *name;

    // Load the OBJ file.

    if ((name = mxmlFindElement(node, node, "file", 0, 0, MXML_DESCEND)))
        file = data->get_obj(std::string(name->child->value.text.string));

    entity::load(node);
}

mxml_node_t *ent::solid::save(mxml_node_t *node)
{
    // Add the OBJ file reference.

    if (file >= 0)
    {
        std::string name = data->get_relative(obj_get_file_name(file));
        mxmlNewText(mxmlNewElement(node, "file"), 0, name.c_str());
    }

    return entity::save(node);
}

mxml_node_t *ent::box::save(mxml_node_t *parent)
{
    // Create a new box element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "box");
    return solid::save(node);
}

mxml_node_t *ent::sphere::save(mxml_node_t *parent)
{
    // Create a new sphere element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "sphere");
    return solid::save(node);
}

mxml_node_t *ent::capsule::save(mxml_node_t *parent)
{
    // Create a new capsule element.

    mxml_node_t *node = mxmlNewElement(parent, "geom");

    mxmlElementSetAttr(node, "class", "capsule");
    return solid::save(node);
}

//-----------------------------------------------------------------------------

