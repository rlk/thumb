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

#include "main.hpp"
#include "util.hpp"
#include "solid.hpp"
#include "entity.hpp"
#include "matrix.hpp"
#include "opengl.hpp"

//-----------------------------------------------------------------------------

#define MAX_CONTACTS 4

dWorldID      ent::entity::world;
dSpaceID      ent::entity::space;
dJointGroupID ent::entity::joint;
dGeomID       ent::entity::point;
dGeomID       ent::entity::focus;

//-----------------------------------------------------------------------------

void ent::entity::phys_pointer(float *dist, dGeomID o1, dGeomID o2)
{
    dContact contact[MAX_CONTACTS];
    int sz = sizeof (dContact);
    int i;
    int n;

    // Note the nearest picking ray collision with a placeable geom.

    if (dGeomGetClass(o2) != dPlaneClass)
    {
        if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
        {
            for (i = 0; i < n; ++i)
            {
                if (*dist > float(contact[i].geom.depth))
                {
                    *dist = float(contact[i].geom.depth);
                    focus = o2;
                }
            }
        }
    }
}

void ent::entity::phys_contact(float *dist, dGeomID o1, dGeomID o2)
{
    // Check for a picking ray collision.

    if      (o1 == point) phys_pointer(dist, o1, o2);
    else if (o2 == point) phys_pointer(dist, o2, o1);
    else
    {
        dBodyID b1 = dGeomGetBody(o1);
        dBodyID b2 = dGeomGetBody(o2);

        // Ignore collisions between geoms associated with the same body.

        if (b1 != b2)
        {
            dContact contact[MAX_CONTACTS];
            int sz = sizeof (dContact);
            int i;
            int n;

            // Check for collisions between these two geoms.
            
            if ((n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz)))
            {
                // Extract the solid entities associated with each geom.

                if (dGeomGetClass(o1) == dGeomTransformClass)
                {
                    set_trg(dGeomGetCategoryBits(o1));
                    o1 = dGeomTransformGetGeom(o1);
                }
                if (dGeomGetClass(o2) == dGeomTransformClass)
                {
                    set_trg(dGeomGetCategoryBits(o2));
                    o2 = dGeomTransformGetGeom(o2);
                }

                entity *e1 = (entity *) dGeomGetData(o1);
                entity *e2 = (entity *) dGeomGetData(o2);

                // Apply the solid surface parameters.

                dSurfaceParameters surface;

                surface.mode = dContactBounce
                             | dContactSoftCFM
                             | dContactSoftERP;

                surface.mu         = dInfinity;
                surface.bounce     = 0.0;
                surface.bounce_vel = 0.1;
                surface.soft_erp   = 1.0;
                surface.soft_cfm   = 0.0;

                e1->get_surface(surface);
                e2->get_surface(surface);

                // Create a contact joint for each collision.

                for (i = 0; i < n; ++i)
                {
                    contact[i].surface = surface;
                    dJointAttach(dJointCreateContact(world, joint,
                                                     contact + i), b1, b2);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

dBodyID ent::entity::phys_body()
{
    // Create and return a new massless body.

    dBodyID body = dBodyCreate(world);
    dMass   mass;

    dMassSetZero(&mass);
    dBodySetMass(body, &mass);

    return body;
}

void ent::entity::phys_init()
{
    // Initialize the physical system.

    world = dWorldCreate();
    space = dHashSpaceCreate(0);
    joint = dJointGroupCreate(0);
    point = dCreateRay(space, 100);
    focus = 0;

    dWorldSetGravity(world, 0, -32, 0);
    dWorldSetAutoDisableFlag(world, 1);
}

void ent::entity::phys_step(float dt)
{
    float dist = 100.0f;

    clr_trg();
    focus = 0;

    // Evaluate the physical system. 

    dSpaceCollide   (space, &dist, (dNearCallback *) phys_contact);
    dWorldQuickStep (world, dt);
    dJointGroupEmpty(joint);
}

void ent::entity::phys_pick(const float p[3], const float v[3])
{
    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

ent::entity *ent::entity::focused()
{
    if (focus)
        return (entity *) dGeomGetData(focus);
    else
        return 0;
}

//-----------------------------------------------------------------------------

ent::entity::entity(int f) : geom(0), body1(0), body2(0), file(f), radius(0),

    lite_prog(glob->get_shader("object-lite")),
    dark_prog(glob->get_shader("object-dark"))

{
    load_idt(default_M);
    load_idt(current_M);

    if (file >= 0) radius = obj_get_file_sphr(file);
}

ent::entity::entity(const entity& that)
{
    std::map<int, param *>::const_iterator i;

    // Copy this object.  Supposedly this won't slice.

    *this = that;

    // Flush and clone each parameter separately.

    params.clear();

    for (i = that.params.begin(); i != that.params.end(); ++i)
        params[i->first] = new param(*i->second);
}

ent::entity::~entity()
{
    // Destroy ODE state.

    if (geom) dGeomDestroy(geom);

    // Delete all parameters.

    std::map<int, param *>::iterator i;
    
    for (i = params.begin(); i != params.end(); ++i)
        delete i->second;
}

//-----------------------------------------------------------------------------

void ent::entity::get_transform(float M[16], dGeomID geom)
{
    const dReal *p = dGeomGetPosition(geom);
    const dReal *R = dGeomGetRotation(geom);

    M[ 0] = float(R[ 0]);
    M[ 1] = float(R[ 4]);
    M[ 2] = float(R[ 8]);
    M[ 3] = 0.0f;

    M[ 4] = float(R[ 1]);
    M[ 5] = float(R[ 5]);
    M[ 6] = float(R[ 9]);
    M[ 7] = 0.0f;

    M[ 8] = float(R[ 2]);
    M[ 9] = float(R[ 6]);
    M[10] = float(R[10]);
    M[11] = 0.0f;

    M[12] = float(p[ 0]);
    M[13] = float(p[ 1]);
    M[14] = float(p[ 2]);
    M[15] = 1.0f;
}

void ent::entity::set_transform(float M[16], dGeomID geom)
{
    dMatrix3 R;

    R[ 0] = (dReal) M[ 0];
    R[ 1] = (dReal) M[ 4];
    R[ 2] = (dReal) M[ 8];
    R[ 3] = (dReal) 0.0f;

    R[ 4] = (dReal) M[ 1];
    R[ 5] = (dReal) M[ 5];
    R[ 6] = (dReal) M[ 9];
    R[ 7] = (dReal) 0.0f;

    R[ 8] = (dReal) M[ 2];
    R[ 9] = (dReal) M[ 6];
    R[10] = (dReal) M[10];
    R[11] = (dReal) 0.0f;

    dGeomSetRotation(geom, R);
    dGeomSetPosition(geom, (dReal) M[12],
                           (dReal) M[13],
                           (dReal) M[14]);
}

void ent::entity::set_default()
{
    memcpy(default_M, current_M, 16 * sizeof (float));
}

void ent::entity::get_default()
{
    memcpy(current_M, default_M, 16 * sizeof (float));

    if (geom) set_transform(current_M, geom);
}

void ent::entity::get_surface(dSurfaceParameters& surface)
{
    std::map<int, param *>::iterator i;

    // Merge this entity's surface parameters with the given structure.

    if ((i = params.find(ent::param::mu))       != params.end())
        surface.mu       = MIN(surface.mu,       (dReal) i->second->value());

    if ((i = params.find(ent::param::bounce))   != params.end())
        surface.bounce   = MAX(surface.bounce,   (dReal) i->second->value());

    if ((i = params.find(ent::param::soft_erp)) != params.end())
        surface.soft_erp = MIN(surface.soft_erp, (dReal) i->second->value());

    if ((i = params.find(ent::param::soft_cfm)) != params.end())
        surface.soft_cfm = MAX(surface.soft_cfm, (dReal) i->second->value());
}

//-----------------------------------------------------------------------------

void ent::entity::turn_world(float a, float vx, float vy, float vz,
                                      float px, float py, float pz)
{
    // Apply the rotation in world space using left-composition.

    Lmul_xlt_inv(current_M, px, py, pz);
    Lmul_rot_mat(current_M, vx, vy, vz, a);
    Lmul_xlt_mat(current_M, px, py, pz);

    if (geom) set_transform(current_M, geom);
}

void ent::entity::turn_local(float a, float vx, float vy, float vz,
                                      float px, float py, float pz)
{
    // Apply the rotation in local space using right-composition.

    Rmul_xlt_mat(current_M, px, py, pz);
    Rmul_rot_mat(current_M, vx, vy, vz, a);
    Rmul_xlt_inv(current_M, px, py, pz);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void ent::entity::turn_world(float a, float vx, float vy, float vz)
{
    // The default center of world rotation is the current world position.

    const float px = current_M[12];
    const float py = current_M[13];
    const float pz = current_M[14];

    // Apply the rotation in world space using left-composition.

    Lmul_xlt_inv(current_M, px, py, pz);
    Lmul_rot_mat(current_M, vx, vy, vz, a);
    Lmul_xlt_mat(current_M, px, py, pz);

    if (geom) set_transform(current_M, geom);
}

void ent::entity::turn_local(float a, float vx, float vy, float vz)
{
    // Apply the rotation in local space using right-composition.

    Rmul_rot_mat(current_M, vx, vy, vz, a);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void ent::entity::move_world(float dx, float dy, float dz)
{
    // Apply the translation in world space using left-composition.

    Lmul_xlt_mat(current_M, dx, dy, dz);

    if (geom) set_transform(current_M, geom);
}

void ent::entity::move_local(float dx, float dy, float dz)
{
    // Apply the translation in local space using right-composition.

    Rmul_xlt_mat(current_M, dx, dy, dz);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void ent::entity::mult_world(const float M[16])
{
    // Apply the given transformation in world space using left-composition.

    mult_mat_mat(current_M, M, current_M);

    if (geom) set_transform(current_M, geom);
}

void ent::entity::mult_local(const float M[16])
{
    // Apply the given transformation in local space using right-composition.

    mult_mat_mat(current_M, M, current_M);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void ent::entity::get_world(float M[16]) const
{
    load_idt(M);

    M[12] = current_M[12];
    M[13] = current_M[13];
    M[14] = current_M[14];
}

void ent::entity::get_local(float M[16]) const
{
    load_mat(M, current_M);
}

float ent::entity::get_bound(float p[3]) const 
{
    p[0] = current_M[12];
    p[1] = current_M[13];
    p[2] = current_M[14];

    return radius;
}

//-----------------------------------------------------------------------------

void ent::entity::edit_fini()
{
    dGeomDestroy(geom);
    geom = 0;
}

//-----------------------------------------------------------------------------

void ent::entity::set_param(int key, std::string& expr)
{
    // Allow only valid parameters as initialized by the entity constructor.

    if (params.find(key) != params.end())
        params[key]->set(expr);
}

bool ent::entity::get_param(int key, std::string& expr)
{
    // Return false to indicate an invalid parameter was requested.

    if (params.find(key) != params.end())
    {
        params[key]->get(expr);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

void ent::entity::mult_M() const
{
    // Apply the current transform as model matrix.

    glMultMatrixf(current_M);
}

void ent::entity::mult_R() const
{
    float M[16];

    // Apply the current view rotation transform.

    M[ 0] = +current_M[ 0];
    M[ 1] = +current_M[ 4];
    M[ 2] = +current_M[ 8];
    M[ 3] = 0.0f;
    M[ 4] = +current_M[ 1];
    M[ 5] = +current_M[ 5];
    M[ 6] = +current_M[ 9];
    M[ 7] = 0.0f;
    M[ 8] = +current_M[ 2];
    M[ 9] = +current_M[ 6];
    M[10] = +current_M[10];
    M[11] = 0.0f;
    M[12] = 0.0f;
    M[13] = 0.0f;
    M[14] = 0.0f;
    M[15] = 1.0f;

    glMultMatrixf(M);
}

void ent::entity::mult_T() const
{
    // Apply the current view translation transform.

    glTranslatef(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
}

void ent::entity::mult_V() const
{
    mult_R();
    mult_T();
}

void ent::entity::mult_P() const
{
}

//-----------------------------------------------------------------------------

void ent::entity::draw_dark()
{
    // Draw the object associated with this entity.

    if (file >= 0)
    {
        glPushMatrix();
        {
            mult_M();

            dark_prog->bind();
            dark_prog->uniform("diffuse", 0);

            obj_draw_file(file);

            glUseProgramObjectARB(0);
        }
        glPopMatrix();
    }
    GL_CHECK();
}

void ent::entity::draw_lite()
{
    // Draw the object associated with this entity.

    if (file >= 0)
    {
        glPushMatrix();
        {
            mult_M();

            lite_prog->bind();
            lite_prog->uniform("diffuse",   0);
            lite_prog->uniform("shadowmap", 1);
            lite_prog->uniform("lightmask", 2);

            obj_draw_file(file);

            glUseProgramObjectARB(0);
        }
        glPopMatrix();
    }
    GL_CHECK();
}

void ent::entity::draw_line()
{
    glPushMatrix();
    {
        // Apply the entity transform.

        mult_M();

        // Highlight active entities in green, inactive in red.

        if (body1)
            glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        else
            glColor4f(1.0f, 0.0f, 0.0f, 0.5f);

        // Draw the wireframe.

        draw_geom();
    }
    glPopMatrix();
}

void ent::entity::draw_foci()
{
    entity *focus = focused();
    bool    draw  = false;

    if (focus == this)
    {
        // Draw the axes at the focused entity.
        
        glPushMatrix();
        {
            mult_M();
            ogl::draw_axes();
        }
        glPopMatrix();
 
        // Highlight the focus in heavy yellow.

        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        glLineWidth(3.0f);

        draw = true;
    }
    else if (focus && focus->body() && focus->body() == body())
    {
        // Highlight the body of the focus in light yellow.

        glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
        glLineWidth(1.0f);

        draw = true;
    }
    else if (focus && focus->join() && focus->join() == body())
    {
        // Highlight the joint target of the focus in light magenta.

        glColor4f(1.0f, 0.0f, 1.0f, 0.5f);
        glLineWidth(1.0f);

        draw = true;
    }

    // Draw the wireframe.

    if (draw)
    {
        glPushMatrix();
        {
            mult_M();
            draw_geom();
        }
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------

void ent::entity::load(mxml_node_t *node)
{
    std::map<int, param *>::iterator i;

    mxml_node_t *n;

    load_idt(current_M);

    float p[3] = { 0, 0, 0    };
    float q[4] = { 0, 0, 0, 1 };

    // Initialize the transform and body mappings.

    for (n = node->child; n; 
         n = mxmlFindElement(n, node, 0, 0, 0, MXML_NO_DESCEND))
    {
        std::string name(n->value.element.name);

        if      (name == "rot_x") q[0] = float(n->child->value.real);
        else if (name == "rot_y") q[1] = float(n->child->value.real);
        else if (name == "rot_z") q[2] = float(n->child->value.real);
        else if (name == "rot_w") q[3] = float(n->child->value.real);

        else if (name == "pos_x") p[0] = float(n->child->value.real);
        else if (name == "pos_y") p[1] = float(n->child->value.real);
        else if (name == "pos_z") p[2] = float(n->child->value.real);

        else if (name == "body1") body1 = n->child->value.integer;
        else if (name == "body2") body2 = n->child->value.integer;
    }

    set_quaternion(current_M, q);

    current_M[12] = p[0];
    current_M[13] = p[1];
    current_M[14] = p[2];

    set_default();

    // Initialize parameters.

    for (i = params.begin(); i != params.end(); ++i)
        i->second->load(node);
}

//-----------------------------------------------------------------------------

mxml_node_t *ent::entity::save(mxml_node_t *node)
{
    std::map<int, param *>::iterator i;

    float q[4];

    // Add the entity transform to this element.

    get_quaternion(q, default_M);

    mxmlNewReal(mxmlNewElement(node, "rot_x"), q[0]);
    mxmlNewReal(mxmlNewElement(node, "rot_y"), q[1]);
    mxmlNewReal(mxmlNewElement(node, "rot_z"), q[2]);
    mxmlNewReal(mxmlNewElement(node, "rot_w"), q[3]);

    mxmlNewReal(mxmlNewElement(node, "pos_x"), default_M[12]);
    mxmlNewReal(mxmlNewElement(node, "pos_y"), default_M[13]);
    mxmlNewReal(mxmlNewElement(node, "pos_z"), default_M[14]);

    // Add entity parameters to this element.

    if (body1) mxmlNewInteger(mxmlNewElement(node, "body1"), body1);
    if (body2) mxmlNewInteger(mxmlNewElement(node, "body2"), body2);

    for (i = params.begin(); i != params.end(); ++i)
        i->second->save(node);

    return node;
}

//-----------------------------------------------------------------------------
