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
#include "joint.hpp"
#include "glob.hpp"
#include "data.hpp"
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

    // Perform collision detection.

    dSpaceCollide2((dGeomID) actor, (dGeomID) scene,
                          this, (dNearCallback *) near_callback);
    dSpaceCollide (actor, this, (dNearCallback *) near_callback);

    // Evaluate the physical system, if requested.

    if (dt > 0.0f)
    {
        dWorldQuickStep (state, dt);
        dJointGroupEmpty(joint);
    }
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

void wrl::world::set_param(int k, std::string& expr)
{
    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
        (*i)->set_param(k, expr);
}

int wrl::world::get_param(int k, std::string& expr)
{
    std::set<std::string> values;

    // Determine the number of distinct values among the selection's params.

    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
        if ((*i)->get_param(k, expr))
            values.insert(expr);

    return int(values.size());
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
    atom_set clones;

    // Remove all selected entities.

    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
        clones.insert((*i)->clone());

    // Select the clones and push an undo-able create operation for them.

    sel = clones;
    do_create();
}

void wrl::world::clear_selection()
{
    sel.clear();
}

bool wrl::world::check_selection()
{
    return !sel.empty();
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

static mxml_type_t load_cb(mxml_node_t *node)
{
    std::string name(node->value.element.name);

    if (name == "param") return MXML_OPAQUE;

    if (name == "world") return MXML_ELEMENT;
    if (name == "joint") return MXML_ELEMENT;
    if (name == "light") return MXML_ELEMENT;
    if (name == "geom")  return MXML_ELEMENT;

    if (name == "body1") return MXML_INTEGER;
    if (name == "body2") return MXML_INTEGER;

    if (name == "rot_x") return MXML_REAL;
    if (name == "rot_y") return MXML_REAL;
    if (name == "rot_z") return MXML_REAL;
    if (name == "rot_w") return MXML_REAL;

    if (name == "pos_x") return MXML_REAL;
    if (name == "pos_y") return MXML_REAL;
    if (name == "pos_z") return MXML_REAL;

    return MXML_TEXT;
}

static const char *save_cb(mxml_node_t *node, int where)
{
    std::string name(node->value.element.name);

    switch (where)
    {
    case MXML_WS_AFTER_OPEN:
        if (name == "?xml")  return "\n";
        if (name == "geom")  return "\n";
        if (name == "joint") return "\n";
        if (name == "world") return "\n";
        if (name == "light") return "\n";
        break;

    case MXML_WS_BEFORE_OPEN:
        if (name == "geom")  return "  ";
        if (name == "joint") return "  ";
        if (name == "light") return "  ";

        if (name == "file")  return "    ";
        if (name == "param") return "    ";
        if (name == "body1") return "    ";
        if (name == "body2") return "    ";

        if (name == "rot_x") return "    ";
        if (name == "rot_y") return "    ";
        if (name == "rot_z") return "    ";
        if (name == "rot_w") return "    ";

        if (name == "pos_x") return "    ";
        if (name == "pos_y") return "    ";
        if (name == "pos_z") return "    ";
        break;

    case MXML_WS_BEFORE_CLOSE:
        if (name == "geom")  return "  ";
        if (name == "joint") return "  ";
        if (name == "light") return "  ";
        break;

    case MXML_WS_AFTER_CLOSE:
        return "\n";
    }

    return NULL;
}

//-----------------------------------------------------------------------------

void wrl::world::init()
{
    sel = all;
    do_delete();
}

void wrl::world::load(std::string name)
{
    // Clear the selection in preparation for selecting all loaded entities.

    sel.clear();

    // Load the named file.

    if (const char *buff = (const char *) ::data->load(name))
    {
        mxml_node_t *n;
        mxml_node_t *H = mxmlLoadString(0, buff, load_cb);
        mxml_node_t *T = mxmlFindElement(H, H, "world", 0, 0,
                                         MXML_DESCEND_FIRST);
        wrl::atom *a;

        // Find all geom elements.

        for (n = mxmlFindElement(T, T, "geom", 0, 0, MXML_DESCEND_FIRST); n;
             n = mxmlFindElement(n, T, "geom", 0, 0, MXML_NO_DESCEND))

            if (mxmlElementGetAttr(n, "class"))
            {
                std::string type(mxmlElementGetAttr(n, "class"));

                // Create a new solid for each recognized geom class.

                if      (type == "box")
                    a =  new wrl::box   (scene, 0);
                else if (type == "sphere")
                    a =  new wrl::sphere(scene, 0);
                else continue;

                // Allow the new solid to parse its own attributes.

                a->load(n);

                // Select the new solid for addition to the scene.

                sel.insert(a);
            }

        // Find all joint elements.

        for (n = mxmlFindElement(T, T, "joint", 0, 0, MXML_DESCEND_FIRST); n;
             n = mxmlFindElement(n, T, "joint", 0, 0, MXML_NO_DESCEND))

            if (mxmlElementGetAttr(n, "type"))
            {
                std::string type(mxmlElementGetAttr(n, "type"));

                // Create a new joint for each recognized joint type.

                if      (type == "ball")
                    a = new  wrl::ball     (state, scene);
                else if (type == "hinge")
                    a = new  wrl::hinge    (state, scene);
                else if (type == "hinge2")
                    a = new  wrl::hinge2   (state, scene);
                else if (type == "slider")
                    a = new  wrl::slider   (state, scene);
                else if (type == "amotor")
                    a = new  wrl::amotor   (state, scene);
                else if (type == "universal")
                    a = new  wrl::universal(state, scene);
                else continue;

                // Allow the new joint to parse its own attributes.

                a->load(n);

                // Select the new joint for addition to the scene.

                sel.insert(a);
            }

        // Add the selected elements to the scene.

        do_create();

        mxmlDelete(H);
    }

    ::data->free(name);
}

void wrl::world::save(std::string filename, bool save_all)
{
    mxml_node_t *head = mxmlNewElement(0, "?xml");
    mxml_node_t *root = mxmlNewElement(head, "world");

    mxmlElementSetAttr(head, "version", "1.0");
    mxmlElementSetAttr(head, "?", 0);

    if (save_all)
        for (atom_set::const_iterator i = all.begin(); i != all.end(); ++i)
            (*i)->save(root);
    else
        for (atom_set::const_iterator i = sel.begin(); i != sel.end(); ++i)
            (*i)->save(root);

    if (char *buff = mxmlSaveAllocString(head, save_cb))
    {
        ::data->save(filename, buff);
        free(buff);
    }
    mxmlDelete(head);
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
