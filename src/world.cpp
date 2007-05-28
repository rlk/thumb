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

#include "odeutil.hpp"
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

wrl::world::world() : serial(1)
{
    // Initialize the editor physical system.

    edit_space = dHashSpaceCreate(0);
    edit_point = dCreateRay(edit_space, 100);
    edit_focus = 0;

    // Initialize the render batchers.

    play_bat = new ogl::batcher();
    play_seg = new ogl::segment();

    edit_bat = new ogl::batcher();
    edit_seg = new ogl::segment();
    
    line_bat = new ogl::batcher();
    pick_seg = new ogl::segment();
    stat_seg = new ogl::segment();
    dyna_seg = new ogl::segment();
    
    play_bat->insert(play_seg);
    edit_bat->insert(edit_seg);
    line_bat->insert(pick_seg);
    line_bat->insert(stat_seg);
    line_bat->insert(dyna_seg);
}

wrl::world::~world()
{
    // Finalize the render batcher.

    delete dyna_seg;
    delete stat_seg;
    delete pick_seg;
    delete line_bat;

    delete edit_seg;
    delete edit_bat;

    delete play_seg;
    delete play_bat;

    // Delete all operations.

    while (!undo_list.empty())
    {
        delete undo_list.front();
        undo_list.pop_front();
    }
    while (!redo_list.empty())
    {
        delete redo_list.front();
        redo_list.pop_front();
    }

    // Finalize the scene.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        delete (*i);

    // Finalize the editor physical system.

    dGeomDestroy (edit_point);
    dSpaceDestroy(edit_space);
}

//-----------------------------------------------------------------------------

void wrl::world::batch_focus(dGeomID curr_focus)
{
    atom *a = curr_focus ? (atom *) dGeomGetData(curr_focus) : 0;
    atom *b = edit_focus ? (atom *) dGeomGetData(edit_focus) : 0;

    ogl::element *A = a ? a->get_line() : 0;
    ogl::element *B = b ? b->get_line() : 0;

    pick_seg->remove(A);

    if (sel.find(a) != sel.end())
    {
        if (a->body())
            dyna_seg->insert(A);
        else
            stat_seg->insert(A);
    }

    dyna_seg->remove(B);
    stat_seg->remove(B);
    pick_seg->insert(B);

    line_bat->dirty();
}

//-----------------------------------------------------------------------------

void wrl::world::edit_callback(dGeomID o1, dGeomID o2)
{
    dContact contact[MAX_CONTACTS];
    int sz = sizeof (dContact);

    dGeomID O1;
    dGeomID O2;

    if      (o1 == edit_point) { O1 = o1; O2 = o2; }
    else if (o2 == edit_point) { O1 = o2; O2 = o1; }
    else return;

    // Note the nearest picking ray collision with a placeable geom.

    if (dGeomGetClass(O2) != dPlaneClass)
    {
        if (int n = dCollide(O1, O2, MAX_CONTACTS, &contact[0].geom, sz))
        {
            for (int i = 0; i < n; ++i)
            {
                if (focus_distance > float(contact[i].geom.depth))
                {
                    focus_distance = float(contact[i].geom.depth);
                    edit_focus = O2;
                }
            }
        }
    }
}

void wrl::world::play_callback(dGeomID o1, dGeomID o2)
{
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);

    // Ignore collisions between geoms associated with the same body.

    if (b1 != b2)
    {
        dContact contact[MAX_CONTACTS];
        int sz = sizeof (dContact);

        // Check for collisions between these two geoms.
            
        if (int n = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sz))
        {
            set_trg(dGeomGetCategoryBits(o1));
            set_trg(dGeomGetCategoryBits(o2));

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

            for (int i = 0; i < n; ++i)
            {
                contact[i].surface = surface;
                dJointAttach(dJointCreateContact(play_world, play_joint,
                                                 contact + i), b1, b2);
            }
        }
    }
}

void edit_callback(wrl::world *that, dGeomID o1, dGeomID o2)
{
    that->edit_callback(o1, o2);
}

void play_callback(wrl::world *that, dGeomID o1, dGeomID o2)
{
    that->play_callback(o1, o2);
}

//-----------------------------------------------------------------------------

void wrl::world::play_init()
{
    int id;

    // Create the world, collision spaces, and joint group.

    play_world = dWorldCreate();
    play_scene = dHashSpaceCreate(0);
    play_actor = dHashSpaceCreate(0);
    play_joint = dJointGroupCreate(0);

    dWorldSetGravity(play_world, 0, -32, 0);
    dWorldSetAutoDisableFlag(play_world, 1);

    // Create a body and segment for each active entity group.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        if ((id = (*i)->body()))
        {
            if (play_body[id] == 0)
            {
                dBodyID body = dBodyCreate(play_world);

                play_body[id] = body;

                // Inilize the mass.

                dMass mass;
                dMassSetZero(&mass);
                dBodySetMass(body, &mass);

                // Initialize the render batcher segment.

                ogl::segment *seg = new ogl::segment;

                play_bat->insert(seg);
                dBodySetData(body, seg);
            }
        }

    // Create a geom for each colliding atoms.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
    {
        dBodyID body = play_body[(*i)->body()];
        dGeomID geom;

        // Add the geom to the correct space.

        if (body)
            geom = (*i)->get_geom(play_actor);
        else
            geom = (*i)->get_geom(play_scene);

        // Attach the geom to its body.

        if (geom) dGeomSetBody(geom, body);

        // Accumulate the body mass.

        if (body)
        {
            dMass total;
            dMass local;

            (*i)->get_mass(&local);
        
            dBodyGetMass(body, &total);
            dMassAdd(&total, &local);
            dBodySetMass(body, &total);
        }
    }

    // Center each body on its center of mass.

    for (body_map::iterator b = play_body.begin(); b != play_body.end(); ++b)
        if (dBodyID body = b->second)
        {
            dMass mass;

            dBodyGetMass(body, &mass);

            dBodySetPosition(body, +mass.c[0], +mass.c[1], +mass.c[2]);
            dMassTranslate (&mass, -mass.c[0], -mass.c[1], -mass.c[2]);

            dBodySetMass(body, &mass);
        }

    // Create and attach all joints.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        if (dJointID join = (*i)->get_join(play_world))
            dJointAttach(join, play_body[(*i)->body()],
                               play_body[(*i)->join()]);

    // Do atom-specific physics initialization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->play_init(play_seg);
}

void wrl::world::play_fini()
{
    // Clean up the render batcher.

    for (body_map::iterator b = play_body.begin(); b != play_body.end(); ++b)
        if (dBodyID body = b->second)
        {
            ogl::segment *seg = (ogl::segment *) dBodyGetData(body);
            play_bat->remove(seg);
            delete seg;
            dBodyDestroy(body);
        }

    play_body.clear();
    play_bat->clear();
    edit_bat->dirty();

    // Do atom-specific physics finalization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->play_fini(edit_seg);

    dJointGroupDestroy(play_joint);
    dSpaceDestroy(play_actor);
    dSpaceDestroy(play_scene);
    dWorldDestroy(play_world);
}

//-----------------------------------------------------------------------------

void wrl::world::edit_pick(const float p[3], const float v[3])
{
    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(edit_point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

void wrl::world::edit_step(float dt)
{
    dGeomID  focus = edit_focus;
    focus_distance = 100;
    edit_focus     =   0;

    // Perform collision detection.

    dSpaceCollide(edit_space, this, (dNearCallback *) ::edit_callback);

    if (focus != edit_focus)
        batch_focus(focus);
}

void wrl::world::play_step(float dt)
{
    // Do atom-specific physics step initialization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->step_init();

    // Perform collision detection.

    clr_trg();

    dSpaceCollide2((dGeomID) play_actor, (dGeomID) play_scene,
                              this, (dNearCallback *) ::play_callback);
    dSpaceCollide(play_actor, this, (dNearCallback *) ::play_callback);

    // Evaluate the physical system.

    dWorldQuickStep (play_world, dt);
    dJointGroupEmpty(play_joint);

    // Transform all segments using current ODE state.

    for (body_map::iterator b = play_body.begin(); b != play_body.end(); ++b)
        if (dBodyID body = b->second)
        {
            ogl::segment *seg = (ogl::segment *) dBodyGetData(body);

            float M[16];

            ode_get_body_transform(body, M);

            seg->move(M);
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

    select_set(op->redo(this));
}

void wrl::world::undo()
{
    // Undo an operation and move it to the redo-able list.

    if (!undo_list.empty())
    {
        select_set(undo_list.front()->undo(this));

        redo_list.push_front(undo_list.front());
        undo_list.pop_front();
    }
}

void wrl::world::redo()
{
    // Redo an operation and move it to the undo-able list.

    if (!redo_list.empty())
    {
        select_set(redo_list.front()->redo(this));

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

    select_set(sel);
}

void wrl::world::clone_selection()
{
    atom_set clones;

    // Remove all selected entities.

    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
        clones.insert((*i)->clone());

    // Select the clones and push an undo-able create operation for them.

    select_set(clones);
    do_create();
}

void wrl::world::clear_selection()
{
    select_set();
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

    select_set(unselected);
}

void wrl::world::extend_selection()
{
    std::set<int> ids;

    // Define a set of the body and join IDs of all selected entities.

    for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
    {
        if ((*i)->body()) ids.insert((*i)->body());
        if ((*i)->join()) ids.insert((*i)->join());
    }

    // Add all entities with an included body or join ID to the selection.

    for (atom_set::iterator j = all.begin(); j != all.end(); ++j)
    {
        if (ids.find((*j)->body()) != ids.end()) sel.insert(*j);
    }

    select_set(sel);
}

//-----------------------------------------------------------------------------

void wrl::world::select_set()
{
    sel.clear();

    // Flush the line batcher.

    pick_seg->clear();
    dyna_seg->clear();
    stat_seg->clear();
    line_bat->dirty();
}

void wrl::world::select_set(atom_set& set)
{
    sel = set;

    // Flush the line batcher.

    pick_seg->clear();
    dyna_seg->clear();
    stat_seg->clear();
    line_bat->dirty();

    // Batch the line elements of all selected atoms.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
        if ((*i)->body())
            dyna_seg->insert((*i)->get_line());
        else
            stat_seg->insert((*i)->get_line());
}

//-----------------------------------------------------------------------------

void wrl::world::create_set(atom_set& set)
{
    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Add the atom's element to the render batcher.

        edit_seg->insert((*i)->get_fill());
        edit_bat->dirty();

        // Add the atom to the atom set.

        all.insert(*i);
        (*i)->live(edit_space);
    }
}

void wrl::world::delete_set(atom_set& set)
{
    // Remove all given atoms from the atom set.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        edit_seg->remove((*i)->get_fill());
        edit_bat->dirty();

        all.erase(all.find(*i));
        (*i)->dead(edit_space);
    }
}

void wrl::world::modify_set(atom_set& set, const float *T)
{
    // Apply the given transform to all given atoms.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
        (*i)->transform(T);

    edit_bat->bind();
    {
        for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
            (*i)->mov_fill();
    }
    edit_bat->free();

    line_bat->bind();
    {
        for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
            (*i)->mov_line();
    }
    line_bat->free();
}

//-----------------------------------------------------------------------------

void wrl::world::do_create()
{
    // Ensure that the new entity body IDs do not conflict with existing ones.

    std::set<int> A;
    std::set<int> B;
    std::set<int> C;

    atom_set::iterator i;
    atom_set::iterator j;

    // Find all conflicting body IDs.

    for (i = all.begin(); i != all.end(); ++i)
        if ((*i)->body()) A.insert((*i)->body());

    for (j = sel.begin(); j != sel.end(); ++j)
        if ((*j)->body()) B.insert((*j)->body());

    std::set_intersection(A.begin(), A.end(),
                          B.begin(), B.end(), std::inserter(C, C.begin()));

    // Generate a new body ID for each conflicting ID.

    std::set<int>::iterator k;
    std::map<int, int>      M;

    for (k = C.begin(); k != C.end(); ++k)
        if (*k) M[*k] = serial++;

    // Remap conflicting IDs.

    for (j = sel.begin(); j != sel.end(); ++j)
    {
        if (M[(*j)->body()]) (*j)->body(M[(*j)->body()]);
        if (M[(*j)->join()]) (*j)->join(M[(*j)->join()]);
    }

    // (This will have nullified all broken joint target IDs.)

    if (!sel.empty()) doop(new ops::create_op(sel));
}

void wrl::world::do_delete()
{
    if (!sel.empty()) doop(new ops::delete_op(sel));
}

void wrl::world::do_enjoin()
{
    if (!sel.empty()) doop(new ops::enjoin_op(sel));
}

void wrl::world::do_embody()
{
    if (!sel.empty()) doop(new ops::embody_op(sel, serial++));
}

void wrl::world::do_debody()
{
    if (!sel.empty()) doop(new ops::embody_op(sel, 0));
}

void wrl::world::do_modify(const float M[16])
{
    if (!sel.empty()) doop(new ops::modify_op(sel, M));
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

    if (name == "body")  return MXML_INTEGER;
    if (name == "join")  return MXML_INTEGER;

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
        if (name == "body")  return "    ";
        if (name == "join")  return "    ";

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
                    a =  new wrl::box   ("");
                else if (type == "sphere")
                    a =  new wrl::sphere("");
                else continue;

                // Allow the new solid to parse its own attributes.

                a->load(n);

                // Select the new solid for addition to the world.

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
                    a = new  wrl::ball     ();
                else if (type == "hinge")
                    a = new  wrl::hinge    ();
                else if (type == "hinge2")
                    a = new  wrl::hinge2   ();
                else if (type == "slider")
                    a = new  wrl::slider   ();
                else if (type == "amotor")
                    a = new  wrl::amotor   ();
                else if (type == "universal")
                    a = new  wrl::universal();
                else continue;

                // Allow the new joint to parse its own attributes.

                a->load(n);

                // Select the new joint for addition to the world.

                sel.insert(a);
            }

        // Add the selected elements to the scene.

        do_create();

        // Ensure the body group serial number does not conflict.

        for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        {
            serial = std::max(serial, (*i)->body() + 1);
            serial = std::max(serial, (*i)->join() + 1);
        }

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

void wrl::world::draw(bool edit)
{
    view->apply();

    if (edit)
    {
        edit_bat->draw_init();
        edit_bat->draw_opaque(false);
        edit_bat->draw_transp(false);
        edit_bat->draw_fini();

        line_init();

        line_bat->draw_init();
        glColor3f(1.0f, 0.0f, 0.0f);
        stat_seg->draw_opaque(false);
        glColor3f(0.0f, 1.0f, 0.0f);
        dyna_seg->draw_opaque(false);
        glColor3f(1.0f, 1.0f, 0.0f);
        pick_seg->draw_opaque(false);
        glColor3f(1.0f, 1.0f, 1.0f);
        line_bat->draw_fini();

        line_fini();
    }
    else
    {
        play_bat->draw_init();
        play_bat->draw_opaque(false);
        play_bat->draw_transp(false);
        play_bat->draw_fini();
    }
}

//-----------------------------------------------------------------------------
