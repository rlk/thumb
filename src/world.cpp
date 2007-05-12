//  Copyright (C) 2007 Robert Kooima
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

#include "matrix.hpp"
#include "world.hpp"
#include "glob.hpp"
#include "view.hpp"
#include "main.hpp"

#define MAX_CONTACTS 4

//-----------------------------------------------------------------------------

wrl::world::world()
{
    atom *a1 = new atom(glob->load_surface("solid/metal_box.obj"),
                        glob->load_surface("wire/wire_box.obj"));

    atom *a2 = new atom(glob->load_surface("solid/metal_box.obj"),
                        glob->load_surface("wire/wire_box.obj"));

    a1->move_world(+1.5, 0.0, 0.0);
    a2->move_world(-1.5, 0.0, 0.0);

    all.insert(a1);
    all.insert(a2);
}

wrl::world::~world()
{
    atom_set::iterator i;

    for (i = all.begin(); i != all.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

void wrl::world::phys_pointer(dGeomID o1, dGeomID o2)
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
                if (focus_distance > float(contact[i].geom.depth))
                {
                    focus_distance = float(contact[i].geom.depth);
                    focus = o2;
                }
            }
        }
    }
}

void wrl::world::phys_contact(dGeomID o1, dGeomID o2)
{
    // Check for a picking ray collision.

    if      (o1 == point) phys_pointer(o1, o2);
    else if (o2 == point) phys_pointer(o2, o1);
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

                atom *a1 = (atom *) dGeomGetData(o1);
                atom *a2 = (atom *) dGeomGetData(o2);

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

                a1->get_surface(surface);
                a2->get_surface(surface);

                // Create a contact joint for each collision.

                for (i = 0; i < n; ++i)
                {
                    contact[i].surface = surface;
                    dJointAttach(dJointCreateContact(state, joint,
                                                     contact + i), b1, b2);
                }
            }
        }
    }
}

void near_callback(wrl::world *that, dGeomID o1, dGeomID o2)
{
    that->phys_contact(o1, o2);
}

//-----------------------------------------------------------------------------

void wrl::world::phys_init()
{
    // Initialize the physical system.

    state = dWorldCreate();
    scene = dHashSpaceCreate(0);
    actor = dHashSpaceCreate(0);
    joint = dJointGroupCreate(0);
    point = dCreateRay(actor, 100);
    focus = 0;

    dWorldSetGravity(state, 0, -32, 0);
    dWorldSetAutoDisableFlag(state, 1);
}

void wrl::world::phys_fini()
{
    dJointGroupDestroy(joint);
    dSpaceDestroy(actor);
    dSpaceDestroy(scene);
    dWorldDestroy(state);
}

void wrl::world::phys_step(float dt)
{
    focus_distance = 100;
    focus          =   0;

    clr_trg();

    // Evaluate the physical system. 

    dSpaceCollide2  ((dGeomID) actor, (dGeomID) scene,
                            this, (dNearCallback *) near_callback);
    dSpaceCollide   (actor, this, (dNearCallback *) near_callback);
    dWorldQuickStep (state, dt);
    dJointGroupEmpty(joint);
}

void wrl::world::phys_pick(const float p[3], const float v[3])
{
    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

wrl::atom *wrl::world::focused() const
{
    if (focus)
        return (atom *) dGeomGetData(focus);
    else
        return 0;
}

//-----------------------------------------------------------------------------

static void line_init()
{
    // Set up for Z-offset anti-aliased line drawing.

    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_OFFSET_LINE);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPolygonOffset(-1.0f, -1.0f);

    glLineWidth(2.0f);

    glUseProgramObjectARB(0);
}

static void line_fini()
{
    glDepthMask(GL_TRUE);

    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

//-----------------------------------------------------------------------------

void wrl::world::draw_scene() const
{
    view->apply();

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->draw_fill();
}

void wrl::world::draw_gizmo() const
{
    line_init();

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->draw_line();

    line_fini();
}

//=============================================================================

wrl::atom::atom(const ogl::surface *f,
                const ogl::surface *l) : geom(0), fill(f), line(l)
{
    load_idt(default_M);
    load_idt(current_M);
}

wrl::atom::~atom()
{
    glob->free_surface(line);
    glob->free_surface(fill);
}

//-----------------------------------------------------------------------------

void wrl::atom::mult_M() const
{
    // Apply the current transform as model matrix.

    glMultMatrixf(current_M);
}

void wrl::atom::mult_R() const
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

void wrl::atom::mult_T() const
{
    // Apply the current view translation transform.

    glTranslatef(-current_M[12],
                 -current_M[13],
                 -current_M[14]);
}

void wrl::atom::mult_V() const
{
    mult_R();
    mult_T();
}

void wrl::atom::mult_P() const
{
}

//-----------------------------------------------------------------------------

void wrl::atom::get_transform(float M[16], dGeomID geom)
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

void wrl::atom::set_transform(float M[16], dGeomID geom)
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

//-----------------------------------------------------------------------------

void wrl::atom::set_default()
{
    memcpy(default_M, current_M, 16 * sizeof (float));
}

void wrl::atom::get_default()
{
    memcpy(current_M, default_M, 16 * sizeof (float));

    if (geom) set_transform(current_M, geom);
}

void wrl::atom::get_surface(dSurfaceParameters& s)
{
    // Merge this atom's surface parameters with the given structure.
}

//-----------------------------------------------------------------------------

void wrl::atom::turn_world(float a, float vx, float vy, float vz,
                                    float px, float py, float pz)
{
    // Apply the rotation in world space using left-composition.

    Lmul_xlt_inv(current_M, px, py, pz);
    Lmul_rot_mat(current_M, vx, vy, vz, a);
    Lmul_xlt_mat(current_M, px, py, pz);

    if (geom) set_transform(current_M, geom);
}

void wrl::atom::turn_local(float a, float vx, float vy, float vz,
                                    float px, float py, float pz)
{
    // Apply the rotation in local space using right-composition.

    Rmul_xlt_mat(current_M, px, py, pz);
    Rmul_rot_mat(current_M, vx, vy, vz, a);
    Rmul_xlt_inv(current_M, px, py, pz);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void wrl::atom::turn_world(float a, float vx, float vy, float vz)
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

void wrl::atom::turn_local(float a, float vx, float vy, float vz)
{
    // Apply the rotation in local space using right-composition.

    Rmul_rot_mat(current_M, vx, vy, vz, a);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void wrl::atom::move_world(float dx, float dy, float dz)
{
    // Apply the translation in world space using left-composition.

    Lmul_xlt_mat(current_M, dx, dy, dz);

    if (geom) set_transform(current_M, geom);
}

void wrl::atom::move_local(float dx, float dy, float dz)
{
    // Apply the translation in local space using right-composition.

    Rmul_xlt_mat(current_M, dx, dy, dz);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void wrl::atom::mult_world(const float M[16])
{
    // Apply the given transformation in world space using left-composition.

    mult_mat_mat(current_M, M, current_M);

    if (geom) set_transform(current_M, geom);
}

void wrl::atom::mult_local(const float M[16])
{
    // Apply the given transformation in local space using right-composition.

    mult_mat_mat(current_M, M, current_M);

    if (geom) set_transform(current_M, geom);
}

//-----------------------------------------------------------------------------

void wrl::atom::get_world(float M[16]) const
{
    load_idt(M);

    M[12] = current_M[12];
    M[13] = current_M[13];
    M[14] = current_M[14];
}

void wrl::atom::get_local(float M[16]) const
{
    load_mat(M, current_M);
}

//-----------------------------------------------------------------------------

void wrl::atom::draw_fill() const
{
    if (fill)
    {
        glPushMatrix();
        {
            mult_M();
            fill->draw(DRAW_OPAQUE | DRAW_UNLIT);
        }
        glPopMatrix();
    }
    
}

void wrl::atom::draw_line() const
{
    glPushMatrix();
    {
        // Apply the entity transform.

        mult_M();

        // Highlight active entities in green, inactive in red.
/*
        if (body1)
            glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
        else
            glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
*/
        // Draw the wireframe.

        line->draw();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------
