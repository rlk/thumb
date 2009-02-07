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

#include <iostream>

#include "ode.hpp"
#include "matrix.hpp"
#include "wrl-world.hpp"
#include "wrl-solid.hpp"
#include "wrl-joint.hpp"
#include "app-glob.hpp"
#include "app-data.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-frustum.hpp"
#include "util.hpp"
#include "main.hpp"

#define MAX_CONTACTS 4

//-----------------------------------------------------------------------------

wrl::world::world() :
    shadow_res(::conf->get_i("shadow_map_resolution")),
    lite_P(45),
    lite_T( 0),
    serial(1)
{
    // Initialize the editor physical system.

    dInitODE();

    edit_space = dHashSpaceCreate(0);
    edit_point = dCreateRay(edit_space, 100);
    edit_focus = 0;

    // Initialize the render pools.

    fill_pool = glob->new_pool();
    fill_node = new ogl::node;

    line_pool = glob->new_pool();
    stat_node = new ogl::node;
    dyna_node = new ogl::node;

    fill_pool->add_node(fill_node);
    line_pool->add_node(stat_node);
    line_pool->add_node(dyna_node);

    // Initialize the lighting state.

    mov_lite(0, 0);
}

wrl::world::~world()
{
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

    dCloseODE();

    // Finalize the render pools.

    glob->free_pool(fill_pool);
    glob->free_pool(line_pool);
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
                if (focus_dist > double(contact[i].geom.depth))
                {
                    focus_dist = double(contact[i].geom.depth);
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

    // Create a body and mass for each active entity group.

    mass_map play_mass;

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        if ((id = (*i)->body()))
        {
            if (play_body[id] == 0)
            {
                dBodyID body = dBodyCreate(play_world);
                dMass   mass;

                dMassSetZero(&mass);

                play_body[id] = body;
                play_mass[id] = mass;

                // Associate the render node.

                dBodySetData(body, nodes[id]);
            }
        }

    // Create a geom for each colliding atoms.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
    {
        id = (*i)->body();

        dBodyID body = play_body[id];
        dGeomID geom;
        dMass   mass;

        // Add the geom to the correct space.

        if (body)
            geom = (*i)->get_geom(play_actor);
        else
            geom = (*i)->get_geom(play_scene);

        if (geom)
        {
            // Attach the geom to its body.

            dGeomSetBody(geom, body);

            // Accumulate the body mass.
            
            if (body)
            {
                (*i)->get_mass(&mass);
                dMassAdd(&play_mass[id], &mass);
            }
        }
    }

    for (body_map::iterator b = play_body.begin(); b != play_body.end(); ++b)
    {
        int     id   = b->first;
        dBodyID body = b->second;
        dMass   mass = play_mass[id];

        if (body)
        {
            // Center the body on its center of mass.

            double M[16];

            load_xlt_mat(M, double(mass.c[0]),
                            double(mass.c[1]),
                            double(mass.c[2]));

            dBodySetPosition(body, +mass.c[0], +mass.c[1], +mass.c[2]);
            dMassTranslate (&mass, -mass.c[0], -mass.c[1], -mass.c[2]);

            dBodySetMass(body, &mass);

            // Recenter the node on the body.

            ((ogl::node *) dBodyGetData(body))->transform(M);
        }
    }

    // Create and attach all joints.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        if (dJointID join = (*i)->get_join(play_world))
            dJointAttach(join, play_body[(*i)->body()],
                               play_body[(*i)->join()]);

    // Do atom-specific physics initialization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->play_init();
}

void wrl::world::play_fini()
{
    double I[16];

    // Reset all node transforms.

    load_idt(I);

    for (node_map::iterator j = nodes.begin(); j != nodes.end(); ++j)
        j->second->transform(I);

    // Do atom-specific physics finalization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->play_fini();

    // Clean up the play-mode physics data.

    play_body.clear();

    dJointGroupDestroy(play_joint);
    dSpaceDestroy(play_actor);
    dSpaceDestroy(play_scene);
    dWorldDestroy(play_world);
}

//-----------------------------------------------------------------------------

void wrl::world::edit_pick(const double *p, const double *v)
{
    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(edit_point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

void wrl::world::edit_step(double dt)
{
    focus_dist = 100;
    edit_focus =   0;

    // Perform collision detection.

    dSpaceCollide(edit_space, this, (dNearCallback *) ::edit_callback);
}

void wrl::world::play_step(double dt)
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
            if (ogl::node *node = (ogl::node *) dBodyGetData(body))
            {
                double M[16];

                ode_get_body_transform(body, M);
                node->transform(M);
            }
}

//-----------------------------------------------------------------------------

void wrl::world::doop(wrl::operation_p op)
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

void wrl::world::mov_lite(int dt, int dp)
{
    // Move the lightsource.  

    lite_P += dp;
    lite_T += dt;

    // Ensure the spherical coordinates remain within bounds.

    if (lite_P >   90) lite_P  =  90;
    if (lite_P <    0) lite_P  =   0;

    if (lite_T >  180) lite_T -= 360;
    if (lite_T < -180) lite_T += 360;

    // Compute the light vector.

    lite_v[0] = sin(RAD(lite_T)) * cos(RAD(lite_P)) * 64.0;
    lite_v[1] =                     sin(RAD(lite_P)) * 64.0;
    lite_v[2] = cos(RAD(lite_T)) * cos(RAD(lite_P)) * 64.0;
}

void wrl::world::node_insert(int id, ogl::unit *unit)
{
    if (id)
    {
        // Ensure that a node exits for this ID.

        if (nodes[id] == 0)
        {
            nodes[id] = new ogl::node();
            fill_pool->add_node(nodes[id]);
        }

        // Add the given unit to the node.

        nodes[id]->add_unit(unit);
            
    }
    else fill_node->add_unit(unit);
}

void wrl::world::node_remove(int id, ogl::unit *unit)
{
    if (id)
    {
        // Remove the unit from its current node.

        nodes[id]->rem_unit(unit);

        // If the node is empty then delete it.

        if (nodes[id]->vcount() == 0)
        {
            delete nodes[id];
            nodes.erase(id);
        }
    }
    else fill_node->rem_unit(unit);
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

    // Flush the line pool.

    stat_node->clear();
    dyna_node->clear();
}

void wrl::world::select_set(atom_set& set)
{
    sel = set;

    // Flush the line pool.

    stat_node->clear();
    dyna_node->clear();

    // Pool the line elements of all selected atoms.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
        if ((*i)->body())
            dyna_node->add_unit((*i)->get_line());
        else
            stat_node->add_unit((*i)->get_line());
}

//-----------------------------------------------------------------------------

void wrl::world::create_set(atom_set& set)
{
    double M[16];

    load_idt(M);

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Add the atom's unit to the render pool.

        node_insert((*i)->body(), (*i)->get_fill());

        // Add the atom to the atom set.

        all.insert(*i);
        (*i)->live(edit_space);

        // Set the default transform.

        (*i)->transform(M);
    }
}

void wrl::world::delete_set(atom_set& set)
{
    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Remove the atom's unit from the render pool.

        node_remove((*i)->body(), (*i)->get_fill());

        // Remove the atom from the atom set.

        all.erase(all.find(*i));
        (*i)->dead(edit_space);
    }
}

void wrl::world::embody_set(atom_set& set, atom_map& map)
{
    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Assign the body ID for each atom.

        node_remove((*i)->body(), (*i)->get_fill());

        (*i)->body(map[*i]);

        node_insert((*i)->body(), (*i)->get_fill());
    }
}

void wrl::world::modify_set(atom_set& set, const double *T)
{
    // Apply the given transform to all given atoms.

    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
        (*i)->transform(T);
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

    if (!sel.empty()) doop(new wrl::create_op(sel));
}

void wrl::world::do_delete()
{
    if (!sel.empty()) doop(new wrl::delete_op(sel));
}

void wrl::world::do_enjoin()
{
    if (!sel.empty()) doop(new wrl::enjoin_op(sel));
}

void wrl::world::do_embody()
{
    if (!sel.empty()) doop(new wrl::embody_op(sel, serial++));
}

void wrl::world::do_debody()
{
    if (!sel.empty()) doop(new wrl::embody_op(sel, 0));
}

void wrl::world::do_modify(const double *M)
{
    if (!sel.empty()) doop(new wrl::modify_op(sel, M));
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

                if      (type == "box")    a = new wrl::box   ("");
                else if (type == "sphere") a = new wrl::sphere("");
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

                if      (type == "ball")      a = new wrl::ball     ();
                else if (type == "hinge")     a = new wrl::hinge    ();
                else if (type == "hinge2")    a = new wrl::hinge2   ();
                else if (type == "slider")    a = new wrl::slider   ();
                else if (type == "amotor")    a = new wrl::amotor   ();
                else if (type == "universal") a = new wrl::universal();
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

void wrl::world::prep_lite(int frusc, app::frustum **frusv)
{
    app::frustum frust(0, shadow_res, shadow_res);
    int          frusi = frusc + 1;

    // Compute a lighting frustum encompasing all view frusta.

    frust.calc_union(frusc, frusv, 0, 1, lite_v, lite_M, lite_I);

    // Cache the fill visibility for the light.

    double d = fill_pool->view(frusi, 5, frust.get_planes());

    // Use the visible range to determine the light projection.
    // TODO: this would benefit from near plane ranging.

//  frust.calc_projection(1.0, std::max(d, 10.0));

    // Render the fill geometry to the shadow buffer.

    if (ogl::binding::bind_shadow_frame(0))
    {
        frust.draw();

        // View from the light's perspective.

        glLoadMatrixd(lite_I);

        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        fill_pool->draw_init();
        {
            glCullFace(GL_FRONT);
            fill_pool->draw(frusi, false, false);
            glCullFace(GL_BACK);
        }
        fill_pool->draw_fini();

        for (int i = 0; i < frusc; ++i)
            frusv[i]->wire();
    }
    ogl::binding::free_shadow_frame(0);

    // Apply the light transform.

    glActiveTextureARB(GL_TEXTURE1);
    glMatrixMode(GL_TEXTURE);
    {
        glLoadIdentity();
        glMultMatrixd(::user->get_S());
        glMultMatrixd(frust.get_P());
        glMultMatrixd(lite_I);
        glMultMatrixd(::user->get_M());
    }
    glMatrixMode(GL_MODELVIEW);
    glActiveTextureARB(GL_TEXTURE0);
}

double wrl::world::prep_fill(int frusc, app::frustum **frusv)
{
    double dist = 10.0;

    // Prep the fill geometry pool.

    fill_pool->prep();

    // Cache the fill visibility and determine the far plane distance.

    for (int frusi = 0; frusi < frusc; ++frusi)
        dist = std::max(dist, fill_pool->view(frusi, 5,
                                              frusv[frusi]->get_planes()));

    // Estimate the corners of all frusta so their union may be found.

    for (int frusi = 0; frusi < frusc; ++frusi)
        frusv[frusi]->calc_view_points(1.0, dist);

    prep_lite(frusc, frusv);

    return dist;
}

double wrl::world::prep_line(int frusc, app::frustum **frusv)
{
    double dist = 0.0;

    // Prep the line geometry pool.

    line_pool->prep();

    // Cache the line visibility and determine the far plane distance.

    for (int frusi = 0; frusi < frusc; ++frusi)
        dist = std::max(dist, line_pool->view(frusi, 5,
                                              frusv[frusi]->get_planes()));
    return dist;
}

void wrl::world::draw_fill(int frusi, app::frustum *frusp)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Compute the light source position.

    GLfloat L[4];

    L[0] = GLfloat(lite_v[0]);
    L[1] = GLfloat(lite_v[1]);
    L[2] = GLfloat(lite_v[2]);
    L[3] = 1.0;

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    // Render the fill geometry.

    fill_pool->draw_init();
    {
        fill_pool->draw(frusi, true, false);
    }
    fill_pool->draw_fini();

    // Render the shadow buffer.

    glUseProgramObjectARB(0);
    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 1.0f);
        ogl::binding::draw_shadow_color(0);
    }
    glPopAttrib();
}

void wrl::world::draw_line(int frusi, app::frustum *frusp)
{
    // Render the line geometry.

    ogl::line_state_init();
    {
        line_pool->draw_init();
        {
            glColor3f(1.0f, 0.0f, 0.0f);
            stat_node->draw(0, true, false);

            glColor3f(0.0f, 1.0f, 0.0f);
            dyna_node->draw(0, true, false);
        }
        line_pool->draw_fini();
    }
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------

/*
double wrl::world::prep(int frusc, app::frustum **frusv, bool edit)
{
    double line_d = 0;

    if (edit)
    {
                 line_pool->prep();
        line_d = line_pool->view(0, 5, planes);
    }
                 fill_pool->prep();
    frust_dist = fill_pool->view(0, 5, planes);

    return std::max(frust_dist, line_d);
    return 0;
}
*/

/*
void wrl::world::draw(int frusi, app::frustum *frusp, bool edit)
{
    GLfloat L[4];
    double  l[4], c[4], d[4];

    const double n =     1000.0; // ::user->get_n(); HACK
    const double f = 10000000.0; // ::user->get_f(); HACK

    const int m = 3;

    // Compute the light source position.

    l[0] = sin(RAD(light_T)) * cos(RAD(light_P));
    l[1] =                     sin(RAD(light_P));
    l[2] = cos(RAD(light_T)) * cos(RAD(light_P));
    l[3] = 0;

    L[0] = GLfloat(l[0]);
    L[1] = GLfloat(l[1]);
    L[2] = GLfloat(l[2]);
    L[3] = 0;

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    fill_pool->draw_init();
    {
        // Compute the splits.

        for (int i = 0; i <= m; ++i)
        {
            double iom = double(i) / double(m);

            c[i] = (n * pow(f / n, iom) + n + (f - n) * iom) / 2;

            d[i] = (n - 1 / c[i]) * f / (f - n);
        }

        // Compute the light projection parameters.

        for (int i = 0; i < m; ++i)
        {
            double M[16], I[16], V[20], a[3], z[3];

            double split[24];

            double kn = (c[i + 0] - n) / (f - n);
            double kf = (c[i + 1] - n) / (f - n);

            LERP(split +  0, points +  0, points + 12, kn);
            LERP(split +  3, points +  3, points + 15, kn);
            LERP(split +  6, points +  6, points + 18, kn);
            LERP(split +  9, points +  9, points + 21, kn);
            LERP(split + 12, points +  0, points + 12, kf);
            LERP(split + 15, points +  3, points + 15, kf);
            LERP(split + 18, points +  6, points + 18, kf);
            LERP(split + 21, points +  9, points + 21, kf);

            get_ortho_light(a, z, M, I, V, l, split);

            float D = fill_pool->view(i + 1, 5, V);

            if (D < 1.0) D = 1.0;

            // Set up the shadow map rendering destination.

            shadow[i]->bind();
            draw_light_init(a[0], z[0], a[1], z[1], -a[2] - D, -a[2], M);
            {
                // Draw the scene to the depth buffer.

                glCullFace(GL_FRONT);
                fill_pool->draw(i + 1, false, false);
                glCullFace(GL_BACK);
            }
            draw_light_fini();
            shadow[i]->free();

            // Set up the shadow projection.

            glActiveTextureARB(GL_TEXTURE1 + i);
            glMatrixMode(GL_TEXTURE);
            {
                glLoadIdentity();

                glMultMatrixd(::user->get_S());
                glOrtho(a[0], z[0], a[1], z[1], -a[2] - D, -a[2]);
                glMultMatrixd(M);
                glMultMatrixd(::user->get_M());
            }
            glMatrixMode(GL_MODELVIEW);
            glActiveTextureARB(GL_TEXTURE0);
        }

        // Set up the shadow map rendering sources.

        shadow[0]->bind_depth(GL_TEXTURE2);
        shadow[1]->bind_depth(GL_TEXTURE3);
        shadow[2]->bind_depth(GL_TEXTURE4);
        {
            ogl::binding::set_split(GLfloat(d[0]),
                                    GLfloat(d[1]),
                                    GLfloat(d[2]),
                                    GLfloat(d[3]));

            // Draw the scene to the color buffer.

            fill_pool->draw(0, true, false);
        }
        shadow[2]->free_depth(GL_TEXTURE4);
        shadow[1]->free_depth(GL_TEXTURE3);
        shadow[0]->free_depth(GL_TEXTURE2);
    }
    fill_pool->draw_fini();

    // Draw the editing widgets.

    if (edit)
    {
        line_init();
        line_pool->draw_init();

        glColor3f(1.0f, 0.0f, 0.0f);
        stat_node->draw(0, true, false);

        glColor3f(0.0f, 1.0f, 0.0f);
        dyna_node->draw(0, true, false);

        line_pool->draw_fini();
        line_fini();
    }
}
*/
//-----------------------------------------------------------------------------
