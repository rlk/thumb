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
#include "solid.hpp"
#include "glob.hpp"
#include "view.hpp"
#include "main.hpp"

#define MAX_CONTACTS 4

//-----------------------------------------------------------------------------

wrl::world::world()
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

    // Initialize the scene.

    float M[16];

    atom *a1 = new box(scene, glob->load_surface("solid/metal_box.obj"));
    atom *a2 = new box(scene, glob->load_surface("solid/metal_box.obj"));

    load_xlt_mat(M, +1.5, 0.0, 0.0);
    a1->mult_world(M);

    load_xlt_mat(M, -1.5, 0.0, 0.0);
    a2->mult_world(M);

    all.insert(a1);
    all.insert(a2);
}

wrl::world::~world()
{
    // Finalize the scene.

    atom_set::iterator i;

    for (i = all.begin(); i != all.end(); ++i)
        delete (*i);

    // Finalize the physical system.

    dJointGroupDestroy(joint);
    dSpaceDestroy(actor);
    dSpaceDestroy(scene);
    dWorldDestroy(state);
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

void wrl::world::pick(const float p[3], const float v[3])
{
    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

void wrl::world::step(float dt)
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

//-----------------------------------------------------------------------------

void wrl::world::doop(ops::operation_p op)
{
    // Delete all redo-able operations.

    while (!redo_list.empty())
    {
        delete redo_list.front();
        redo_list.pop_front();
    }

    // Add this operation to the undo-able list.

    undo_list.push_front(op);

    // Do the operation for the first time.

    op->redo(this);
}

void wrl::world::undo()
{
    // Undo an operation and move it to the redo-able list.

    if (!undo_list.empty())
    {
        sel = undo_list.front()->undo(this);

        redo_list.push_front(undo_list.front());
        undo_list.pop_front();
    }
}

void wrl::world::redo()
{
    // Redo an operation and move it to the undo-able list.

    if (!redo_list.empty())
    {
        sel = redo_list.front()->redo(this);

        undo_list.push_front(redo_list.front());
        redo_list.pop_front();
    }
}

//-----------------------------------------------------------------------------

void wrl::world::click_selection(atom *a)
{
    if (sel.find(a) != sel.end())
        sel.erase(a);
    else
        sel.insert(a);
}

void wrl::world::clone_selection()
{
}

void wrl::world::clear_selection()
{
    sel.clear();
}

//-----------------------------------------------------------------------------

void wrl::world::invert_selection()
{
    // Begin with the set of all entities.

    atom_set unselected;
    atom_set::iterator i;

    unselected = all;

    // Remove all selected entities.

    for (i = sel.begin(); i != sel.end(); ++i)
        unselected.erase(unselected.find(*i));

    // Giving the set of all unselected entities.

    sel = unselected;
}

void wrl::world::extend_selection()
{
}

//-----------------------------------------------------------------------------

void wrl::world::create_set(atom_set& set)
{
    // Add all given atoms to the atom set.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        all.insert(*i);
    }
}

void wrl::world::delete_set(atom_set& set)
{
    // Remove all given atoms from the atom set.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        all.erase(all.find(*i));
    }
}

void wrl::world::modify_set(atom_set& set, const float T[16])
{
    // Apply the given transform to all given atoms.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        (*i)->mult_world(T);
        (*i)->set_default();
    }
}

//-----------------------------------------------------------------------------

void wrl::world::do_create()
{
    if (!sel.empty())
    {
        doop(new ops::create_op(sel));
    }
}

void wrl::world::do_delete()
{
    if (!sel.empty())
    {
        doop(new ops::delete_op(sel));
        sel.clear();
    }
}

void wrl::world::do_modify(const float M[16])
{
    if (!sel.empty())
    {
        doop(new ops::modify_op(sel, M));
    }
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

    glPolygonOffset(-1.1f, -4.0f);

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
    view->apply();

    line_init();

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->draw_foci(focus);
    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
        (*i)->draw_stat();

    line_fini();
}

//-----------------------------------------------------------------------------
