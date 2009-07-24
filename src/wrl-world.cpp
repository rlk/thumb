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
#include <algorithm>

#include "ode.hpp"
#include "util.hpp"
#include "main.hpp"
#include "matrix.hpp"
#include "ogl-pool.hpp"
#include "ogl-uniform.hpp"
#include "ogl-process.hpp"
#include "ogl-terrain.hpp"
#include "app-prog.hpp"
#include "app-glob.hpp"
#include "app-data.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-file.hpp"
#include "app-frustum.hpp"
#include "wrl-solid.hpp"
#include "wrl-joint.hpp"
#include "wrl-world.hpp"

#define MAX_CONTACTS 4

//-----------------------------------------------------------------------------

wrl::world::world() :
    shadow_res(::conf->get_i("shadow_map_resolution")),

    split_static(false),
    land(0),

    sky      (::glob->load_binding("sky-water",       "sky-water")),
    sky_light(::glob->load_binding("sky-water-light", "sky-water-light")),
    sky_shade(::glob->load_binding("sky-water-shade", "sky-water-shade")),
    serial(1)
{
    // Initialize the editor physical system.

    dInitODE();

    edit_space = dHashSpaceCreate(0);
    edit_point = dCreateRay(edit_space, 100);
    edit_focus = 0;

    // Initialize the render pools.

    fill_pool = ::glob->new_pool();
    fill_node = new ogl::node;

    line_pool = ::glob->new_pool();
    stat_node = new ogl::node;
    dyna_node = new ogl::node;

    fill_pool->add_node(fill_node);
    line_pool->add_node(stat_node);
    line_pool->add_node(dyna_node);

    // Initialize the render uniforms and processes.

    uniform_light_position = ::glob->load_uniform("light_position",    3);
    uniform_pssm_depth     = ::glob->load_uniform("pssm_depth",        4);
    uniform_shadow[0]      = ::glob->load_uniform("shadow_matrix[0]", 16);
    uniform_shadow[1]      = ::glob->load_uniform("shadow_matrix[1]", 16);
    uniform_shadow[2]      = ::glob->load_uniform("shadow_matrix[2]", 16);
    process_shadow[0]      = ::glob->load_process("shadow",            0);
    process_shadow[1]      = ::glob->load_process("shadow",            1);
    process_shadow[2]      = ::glob->load_process("shadow",            2);
    process_reflection[0]  = ::glob->load_process("reflection_env",    0);
    process_reflection[1]  = ::glob->load_process("reflection_env",    1);
    process_irradiance[0]  = ::glob->load_process("irradiance_env",    0);
    process_irradiance[1]  = ::glob->load_process("irradiance_env",    1);

//  click_selection(new wrl::box("solid/bunny.obj"));
//  click_selection(new wrl::box("solid/buddha.obj"));
//  do_create();
}

wrl::world::~world()
{
    // Atoms own units, so units must be removed from nodes before deletion.

    fill_node->clear();
    stat_node->clear();
    dyna_node->clear();

    for (node_map::iterator j = nodes.begin(); j != nodes.end(); ++j)
        j->second->clear();

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

    // Finalize the atoms.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        delete (*i);

    // Finalize the editor physical system.

    dGeomDestroy (edit_point);
    dSpaceDestroy(edit_space);

    dCloseODE();

    // Finalize the uniforms.

    ::glob->free_process(process_irradiance[1]);
    ::glob->free_process(process_irradiance[0]);
    ::glob->free_process(process_reflection[1]);
    ::glob->free_process(process_reflection[0]);
    ::glob->free_process(process_shadow[2]);
    ::glob->free_process(process_shadow[1]);
    ::glob->free_process(process_shadow[0]);
    ::glob->free_uniform(uniform_shadow[2]);
    ::glob->free_uniform(uniform_shadow[1]);
    ::glob->free_uniform(uniform_shadow[0]);
    ::glob->free_uniform(uniform_pssm_depth);
    ::glob->free_uniform(uniform_light_position);

    // Finalize the render pools.

    ::glob->free_pool(fill_pool);
    ::glob->free_pool(line_pool);

    ::glob->free_terrain(land);

    // Finalize the sky materials.

    ::glob->free_binding(sky_shade);
    ::glob->free_binding(sky_light);
    ::glob->free_binding(sky);
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
            fill_pool->rem_node(nodes[id]);
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

void wrl::world::init()
{
    sel = all;
    do_delete();
}

void wrl::world::load(std::string name)
{
    wrl::atom *a;

    // Clear the selection in preparation for selecting all loaded entities.

    sel.clear();

    // Load the named file.

    app::file file(name);
    app::node root(file.get_root().find("world"));

    // Check for static split definitions.

    split_static = (((split[0] = root.get_f("split0", 0.0)) > 0.0) |
                    ((split[1] = root.get_f("split1", 0.0)) > 0.0) |
                    ((split[2] = root.get_f("split2", 0.0)) > 0.0) |
                    ((split[3] = root.get_f("split3", 0.0)) > 0.0));

    // Check for a terrain definition.

    std::string land_name = root.get_s("land");

    if (!land_name.empty())
        land = ::glob->load_terrain(land_name);

    // Find all geom elements.

    for (app::node n = root.find("geom"); n; n = root.next(n, "geom"))
    {
        std::string type = n.get_s("class");

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

    for (app::node n = root.find("joint"); n; n = root.next(n, "joint"))
    {
        std::string type = n.get_s("type");

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
}

void wrl::world::save(std::string filename, bool save_all)
{
    // Construct a new XML DOM using from the current world.

    app::node head("?xml");
    app::node body("world");

    head.set_s("version", "1.0");
    head.set_s("?", "");
    
    body.insert(head);

    // Add some or all atoms to the DOM, as requested.

    if (save_all)
        for (atom_set::const_iterator i = all.begin(); i != all.end(); ++i)
            (*i)->save(body);
    else
        for (atom_set::const_iterator i = sel.begin(); i != sel.end(); ++i)
            (*i)->save(body);

    // Write the DOM to the named file.

    head.write(filename);
}

//-----------------------------------------------------------------------------

ogl::range wrl::world::prep_fill(int frusc, const app::frustum *const *frusv)
{
    // Position the light source.

    const double *L = ::user->get_L();

    light[0] = L[0];
    light[1] = L[1];
    light[2] = L[2];

    // Prep the fill geometry pool.

    fill_pool->prep();

    // Cache the fill visibility and determine the far plane distance.

    ogl::range r;

    for (int frusi = 0; frusi < frusc; ++frusi)
    {
        r.merge(fill_pool->view(frusi, 5, frusv[frusi]->get_planes()));

        if (land)
            r.merge(land->view(frusv[frusi]->get_planes(), 5));
    }

    return r;
}

ogl::range wrl::world::prep_line(int frusc, const app::frustum *const *frusv)
{
    ogl::range r;

    // Prep the line geometry pool.

    line_pool->prep();

    // Cache the line visibility and determine the far plane distance.

    for (int frusi = 0; frusi < frusc; ++frusi)
        r.merge(line_pool->view(frusi, 5, frusv[frusi]->get_planes()));

    return r;
}

//-----------------------------------------------------------------------------

double wrl::world::split_fract(int i, int m, const app::frustum *frusp)
{
    double c;

    if (split_static)
        c = split[i];
    else
        c = frusp->get_split_coeff(double(i) / double(m));
    
    return frusp->get_split_fract(c);
}

double wrl::world::split_depth(int i, int m, const app::frustum *frusp)
{
    double c;

    if (split_static)
        c = split[i];
    else
        c = frusp->get_split_coeff(double(i) / double(m));
    
    return frusp->get_split_depth(c);
}

void wrl::world::lite(int frusc, const app::frustum *const *frusv)
{
    double lite_M[16];
    double lite_I[16];

    // Compute the shadow map split depths.

    int m = 3; // TODO: generalize split count

    double c[4];
    double d[4];

    // TODO: is the first always sufficient?  Maybe MAX, MIN, or AVG?

    for (int i = 0; i <= m; ++i)
    {
        c[i] = split_fract(i, m, frusv[0]);
        d[i] = split_depth(i, m, frusv[0]);
    }

    uniform_pssm_depth->set(d);

    // Render each of the shadow maps.

    for (int i = 0; i < m; ++i)
    {
        app::frustum frust(0, shadow_res, shadow_res);
        int          frusi = frusc + i + 1;

        // Compute a lighting frustum encompasing all view frusta.

        frust.calc_union(frusc, frusv, c[i], c[i+1], light, lite_M, lite_I);

        // Cache the fill visibility for the light.

        ogl::range s = fill_pool->view(frusi, 5, frust.get_planes());

        // Use the visible range to determine the light projection.
        // TODO: optimize usinge calc_union's discovered range.

        frust.set_distances(s.get_n(), s.get_f());

        // Render the fill geometry to the shadow buffer.

//      glEnable(GL_POLYGON_OFFSET_FILL);
        process_shadow[i]->bind_frame();
        {
//          glPolygonOffset(1.1f, 4.0f);
            frust.draw();

            // View from the light's perspective.

            glLoadMatrixd(lite_I);

            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);

            // NOTE: Uniforms have NOT been refreshed at this point.  The
            // previous frame's uniforms are current.

            fill_pool->draw_init();
            {
                glCullFace(GL_FRONT);
                fill_pool->draw(frusi, false, false);
                fill_pool->draw(frusi, false, true);
                glCullFace(GL_BACK);
            }
            fill_pool->draw_fini();
        }
        process_shadow[i]->free_frame();
//      glDisable(GL_POLYGON_OFFSET_FILL);

        // Compute the light transform.

        double M[16];

        load_mat    (M,    ::user->get_S()); // TODO: eliminate the use of ::user here.
        mult_mat_mat(M, M, frust.get_P());
        mult_mat_mat(M, M, lite_I);
        mult_mat_mat(M, M, ::user->get_M());

        uniform_shadow[i]->set(M);
    }

    // Ask the binding system to compute the irradiance environment using
    // the sky shader.

    process_reflection[0]->draw(sky_shade);
    process_reflection[1]->draw(sky_light);
    process_irradiance[0]->draw(0);
    process_irradiance[1]->draw(0);
}

//-----------------------------------------------------------------------------

void wrl::world::draw_fill(int frusi, const app::frustum *frusp)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Compute the light source position.

    GLfloat L[4];

    L[0] = GLfloat(light[0]);
    L[1] = GLfloat(light[1]);
    L[2] = GLfloat(light[2]);
    L[3] = 1.0f;

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    // Render the terrain.

    if (land)
        land->draw(frusp->get_view_pos(), frusp->get_planes(), 5);

    // Render the fill geometry.

    glAlphaFunc(GL_GREATER, 0.5f);

    fill_pool->draw_init();
    {
        fill_pool->draw(frusi, true, false);
        fill_pool->draw(frusi, true, true);
    }
    fill_pool->draw_fini();

    // Render the sky.

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_sky(frusp);

    if (::prog->get_option(4)) draw_debug_wireframe(frusi);
}

void wrl::world::draw_line(int frusi, const app::frustum *frusp)
{
    // Render the line geometry.

    ogl::line_state_init();
    {
        line_pool->draw_init();
        {
            glColor3f(1.0f, 0.0f, 0.0f);
            stat_node->draw(frusi, true, false);

            glColor3f(0.0f, 1.0f, 0.0f);
            dyna_node->draw(frusi, true, false);

            glColor3f(1.0f, 1.0f, 1.0f);
        }
        line_pool->draw_fini();
    }
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------

void wrl::world::draw_sky(const app::frustum *frusp)
{
    const double *vp = frusp->get_view_pos();
    const double *v0 = frusp->get_points() + 0;
    const double *v1 = frusp->get_points() + 3;
    const double *v2 = frusp->get_points() + 6;
    const double *v3 = frusp->get_points() + 9;

    sky->bind(true);

    glEnable(GL_POLYGON_OFFSET_FILL);
    {
        // Draw the far plane of the clip space, offset by one unit of
        // depth buffer distance.  Pass the world-space vectors from the
        // view position toward the screen corners for use in sky display.

        glPolygonOffset(0.0, -1.0);

        glBegin(GL_QUADS);
        {
            glTexCoord2d(0, 0);
            glVertex3d(v0[0] - vp[0], v0[1] - vp[1], v0[2] - vp[2]);
            glTexCoord2d(1, 0);
            glVertex3d(v1[0] - vp[0], v1[1] - vp[1], v1[2] - vp[2]);
            glTexCoord2d(1, 1);
            glVertex3d(v3[0] - vp[0], v3[1] - vp[1], v3[2] - vp[2]);
            glTexCoord2d(0, 1);
            glVertex3d(v2[0] - vp[0], v2[1] - vp[1], v2[2] - vp[2]);
        }
        glEnd();
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void wrl::world::draw_debug_wireframe(int frusi)
{
    // Render the fill geometry in wireframe.

    ogl::line_state_init();
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glLineWidth(1.0);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        {
            fill_pool->draw_init();
            {
                fill_pool->draw(frusi, true, false);
                fill_pool->draw(frusi, true, true);
            }
            fill_pool->draw_fini();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------
