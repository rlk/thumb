//  Copyright (C) 2007-2011 Robert Kooima
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

#include <algorithm>
#include <iterator>
#include <iostream>
#include <cassert>

#include <etc-vector.hpp>
#include <etc-ode.hpp>
#include <ogl-pool.hpp>
#include <ogl-uniform.hpp>
#include <ogl-process.hpp>
#include <app-glob.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-file.hpp>
#include <app-frustum.hpp>
#include <wrl-solid.hpp>
#include <wrl-light.hpp>
#include <wrl-joint.hpp>
#include <wrl-world.hpp>

#define MAX_CONTACTS 4

//-----------------------------------------------------------------------------

wrl::world::world() :
    shadow_res(::conf->get_i("shadow_map_resolution", 1024)),

    sky(::glob->load_binding("sky-water", "sky-water")),
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
    line_node = new ogl::node;

    fill_pool->add_node(fill_node);
    line_pool->add_node(line_node);

    // Initialize the render uniforms and processes.

    uniform_shadow[0]      = ::glob->load_uniform("ShadowMatrix[0]", 16);
    uniform_shadow[1]      = ::glob->load_uniform("ShadowMatrix[1]", 16);
    uniform_shadow[2]      = ::glob->load_uniform("ShadowMatrix[2]", 16);
    uniform_shadow[3]      = ::glob->load_uniform("ShadowMatrix[3]", 16);
    process_shadow[0]      = ::glob->load_process("shadow",           0);
    process_shadow[1]      = ::glob->load_process("shadow",           1);
    process_shadow[2]      = ::glob->load_process("shadow",           2);
    process_shadow[3]      = ::glob->load_process("shadow",           3);

//  click_selection(new wrl::box("solid/bunny.obj"));
//  click_selection(new wrl::box("solid/buddha.obj"));
//  do_create();
}

wrl::world::~world()
{
    // Atoms own units, so units must be removed from nodes before deletion.

    fill_node->clear();
    line_node->clear();

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

    ::glob->free_process(process_shadow[3]);
    ::glob->free_process(process_shadow[2]);
    ::glob->free_process(process_shadow[1]);
    ::glob->free_process(process_shadow[0]);
    ::glob->free_uniform(uniform_shadow[3]);
    ::glob->free_uniform(uniform_shadow[2]);
    ::glob->free_uniform(uniform_shadow[1]);
    ::glob->free_uniform(uniform_shadow[0]);

    // Finalize the render pools.

    ::glob->free_pool(fill_pool);
    ::glob->free_pool(line_pool);

    // Finalize the sky materials.

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
            /* TODO: Reimplement collision triggers with cluster awareness.
            set_trg(dGeomGetCategoryBits(o1));
            set_trg(dGeomGetCategoryBits(o2));
            */

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

    // Create a geom for each colliding atom.

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

            mat4 M = translation(vec3(double(mass.c[0]),
                                      double(mass.c[1]),
                                      double(mass.c[2])));

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
    // Reset all node transforms.

    for (node_map::iterator j = nodes.begin(); j != nodes.end(); ++j)
        j->second->transform(mat4());

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

void wrl::world::edit_pick(const vec3& p, const vec3& v)
{
    // These assertions head off a variety of difficult-to-track issues.

    assert(!std::isnan(p[0]));
    assert(!std::isnan(p[1]));
    assert(!std::isnan(p[2]));
    assert(!std::isnan(v[0]));
    assert(!std::isnan(v[1]));
    assert(!std::isnan(v[2]));

    // Apply the pointer position and vector to the picking ray.

    dGeomRaySet(edit_point, p[0], p[1], p[2], v[0], v[1], v[2]);
}

void wrl::world::edit_step(double dt)
{
    // Perform collision detection.

    focus_dist = 100;
    edit_focus =   0;
    dSpaceCollide(edit_space, this, (dNearCallback *) ::edit_callback);
}

void wrl::world::play_step(double dt)
{
    // Do atom-specific physics step initialization.

    for (atom_set::iterator i = all.begin(); i != all.end(); ++i)
        (*i)->step_init();

    // Perform collision detection.

    // TODO: move clr_trg somewhere
    // clr_trg();

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
                node->transform(ode_get_body_transform(body));
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

void wrl::world::node_insert(int id, ogl::unit *fill, ogl::unit *line)
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

        nodes[id]->add_unit(fill);
    }
    else fill_node->add_unit(fill);

    line_node->add_unit(line);
}

void wrl::world::node_remove(int id, ogl::unit *fill, ogl::unit *line)
{
    if (id)
    {
        // Remove the unit from its current node.

        nodes[id]->rem_unit(fill);

        // If the node is empty then delete it.

        if (nodes[id]->vcount() == 0)
        {
            fill_pool->rem_node(nodes[id]);
            delete nodes[id];
            nodes.erase(id);
        }
    }
    else fill_node->rem_unit(fill);

    line_node->rem_unit(line);
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
}

void wrl::world::select_set(atom_set& set)
{
    sel = set;
}

//-----------------------------------------------------------------------------

void wrl::world::create_set(atom_set& set)
{
    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Add the atom's unit to the render pool.

        node_insert((*i)->body(), (*i)->get_fill(), (*i)->get_line());

        // Add the atom to the atom set.

        all.insert(*i);
        (*i)->live(edit_space);

        // Set the default transform.

        (*i)->transform(mat4());
    }
}

void wrl::world::delete_set(atom_set& set)
{
    for (atom_set::iterator i = set.begin(); i != set.end(); ++i)
    {
        // Remove the atom's unit from the render pool.

        node_remove((*i)->body(), (*i)->get_fill(), (*i)->get_line());

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

        node_remove((*i)->body(), (*i)->get_fill(), (*i)->get_line());

        (*i)->body(map[*i]);

        node_insert((*i)->body(), (*i)->get_fill(), (*i)->get_line());
    }
}

void wrl::world::modify_set(atom_set& set, const mat4& T)
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

void wrl::world::do_modify(const mat4& M)
{
    if (!sel.empty()) doop(new wrl::modify_op(sel, M));
}

//-----------------------------------------------------------------------------

void wrl::world::init()
{
    sel = all;
    do_delete();
    edit_focus = 0;
}

void wrl::world::load(std::string name)
{
    wrl::atom *a;

    // Clear the selection in preparation for selecting all loaded entities.

    sel.clear();

    // Load the named file.

    try
    {
        app::file file(name);
        app::node root(file.get_root().find("world"));

        // Find all geom elements.

        for (app::node n = root.find("geom"); n; n = root.next(n, "geom"))
        {
            std::string type = n.get_s("type");

            // Create a new solid for each recognized geom type.

            if      (type == "box")     a = new wrl::box   ("");
            else if (type == "sphere")  a = new wrl::sphere("");
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

        // Find all light elements.

        for (app::node n = root.find("light"); n; n = root.next(n, "light"))
        {
            std::string type = n.get_s("type");

            // Create a new light for each recognized light type.

            if      (type == "d-light") a = new wrl::d_light();
            else if (type == "s-light") a = new wrl::s_light();
            else continue;

            // Allow the new light to parse its own attributes.

            a->load(n);

            // Select the new light for addition to the world.

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
    catch (std::exception& e)
    {
        std::cerr << "world::load: " << e.what() << std::endl;
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

ogl::aabb wrl::world::prep_fill(int frusc, const app::frustum *const *frusv)
{
    // Prep the fill geometry pool.

    fill_pool->prep();

    // Cache the fill visibility and determine the visible bound.

    ogl::aabb bb;

    for (int frusi = 0; frusi < frusc; ++frusi)
        bb.merge(fill_pool->view(frusi, frusv[frusi]->get_world_planes(), 5));

    return bb;
}

ogl::aabb wrl::world::prep_line(int frusc, const app::frustum *const *frusv)
{
    // Prep the line geometry pool.

    line_pool->prep();

    // Cache the line visibility and determine the visible bound.

    ogl::aabb bb;

    for (int frusi = 0; frusi < frusc; ++frusi)
        bb.merge(line_pool->view(frusi, frusv[frusi]->get_world_planes(), 5));

    return bb;
}

//-----------------------------------------------------------------------------

void wrl::world::shadow(int frusi, app::frustum *frusp, int light)
{
    // Bound the frustum to its visible volume.

    ogl::aabb b = fill_pool->view(frusi, frusp->get_world_planes(), 5);

    frusp->set_bound(mat4(), b);

    // Render the fill geometry to the shadow buffer.

    process_shadow[light]->bind_frame();
    {
        frusp->load_transform();

        glLoadIdentity();
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT);

        // NOTE: Uniforms have NOT been refreshed at this point.

        fill_pool->draw_init();
        {
            glCullFace(GL_FRONT);
            fill_pool->draw(frusi, false, false);
            fill_pool->draw(frusi, false, true);
            glCullFace(GL_BACK);
        }
        fill_pool->draw_fini();
    }
    process_shadow[light]->free_frame();

    // Compute the light transform.

    const mat4 light_S(0.5, 0.0, 0.0, 0.5,
                       0.0, 0.5, 0.0, 0.5,
                       0.0, 0.0, 0.5, 0.5,
                       0.0, 0.0, 0.0, 1.0);

    uniform_shadow[light]->set(light_S * frusp->get_transform()
                                      * ::view->get_inverse());
}

int wrl::world::s_light(const vec3& p, const vec3& v,
                        int frusc, const app::frustum *const *frusv,
                        int light, const ogl::aabb& visible, atom *a)
{
    double f = 2.0 * a->cache_light(light, vec4(p, 1), vec4(-v, 0), 0, 1);

    app::perspective_frustum frust(p, -v, f, 1.0);
    shadow(frusc + light, &frust, light);

    return 1;
}

int wrl::world::d_light(const vec3& p, const vec3& v,
                        int frusc, const app::frustum *const *frusv,
                        int light, const ogl::aabb& visible, atom *a)
{
    int n = 3;

    for (int i = 0; i < n; i++, light++)
    {
        a->cache_light(light, vec4(v, 0), vec4(v, 0), i, n);

        // Compute the visible union of the bounds of this split.

        ogl::aabb bound;

        for (int frusi = 0; frusi < frusc; ++frusi)
            bound.merge(frusv[frusi]->get_split_bound(i, n));

        bound.intersect(visible);

        // Render a shadow map encompasing this bound.

        app::orthogonal_frustum frust(bound, v);
        shadow(frusc + light, &frust, light);
    }
    return n;
}

void wrl::world::lite(int frusc, const app::frustum *const *frusv)
{
    // Determine the visible bounding volume. TODO: Remove this redundancy.

    ogl::aabb bb;

    for (int frusi = 0; frusi < frusc; ++frusi)
        bb.merge(fill_pool->view(frusi, frusv[frusi]->get_world_planes(), 5));

    // Enumerate the light sources.

    int light = 0;

    atom_set::iterator a;

    for (a = all.begin(); a != all.end() && (*a)->priority() < 0; ++a)
    {
        const mat4 T = (*a)->get_fill()->get_world_transform();
        const vec3 p = wvector(T);
        const vec3 v = yvector(T);

        if ((*a)->priority() == -1)
            light += s_light(p, v, frusc, frusv, light, bb, *a);
        if ((*a)->priority() == -2)
            light += d_light(p, v, frusc, frusv, light, bb, *a);
    }

    // Zero the unused lights.

    GLfloat Z[4] = { 0, 0, 0, 0 };

    for (; light < ogl::max_lights; light++)
    {
        glLightfv(GL_LIGHT0 + light, GL_POSITION, Z);
        glLightfv(GL_LIGHT0 + light, GL_DIFFUSE,  Z);
    }
}

//-----------------------------------------------------------------------------

void wrl::world::draw_fill(int frusi, const app::frustum *frusp)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the fill geometry.

    draw_sky(frusp);

    glAlphaFunc(GL_GREATER, 0.5f);

    fill_pool->draw_init();
    {
        fill_pool->draw(frusi, true, false);
        fill_pool->draw(frusi, true, true);
    }
    fill_pool->draw_fini();
}

void wrl::world::draw_lite()
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render the light geometry.

    glDepthFunc(GL_EQUAL);
    fill_pool->draw_init();
    {
        atom_set::const_iterator a;

        for (a = all.begin(); a != all.end() && (*a)->priority() < 0; ++a)
        {
            (*a)->apply_light(0);
            (*a)->get_fill()->draw_faces();
        }
    }
    fill_pool->draw_fini();
    glDepthFunc(GL_LESS);
}

void wrl::world::draw_line()
{
    // Render the line geometry.

    ogl::line_state_init();
    {
        line_pool->draw_init();
        {
            // Render the current selection.

            for (atom_set::iterator i = sel.begin(); i != sel.end(); ++i)
            {
                if ((*i)->body())
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(1.0f, 0.0f, 0.0f);

                glLineWidth(4.0);
                (*i)->get_line()->draw_lines();
            }

            // Highlight the current focus.

            if (edit_focus)
            {
                if (wrl::atom *a = (wrl::atom *) dGeomGetData(edit_focus))
                {
                    glColor3f(1.0f, 1.0f, 0.0f);
                    glLineWidth(2.0);
                    a->get_line()->draw_lines();
                }
            }

            glColor3f(1.0f, 1.0f, 1.0f);
        }
        line_pool->draw_fini();
    }
    ogl::line_state_fini();
}

//-----------------------------------------------------------------------------

void wrl::world::draw_sky(const app::frustum *frusp)
{
    sky->bind(true);

    glEnable(GL_POLYGON_OFFSET_FILL);
    {
        const vec3 *v = frusp->get_world_points();
        const vec3 v0 = v[4] - v[0];
        const vec3 v1 = v[5] - v[1];
        const vec3 v2 = v[6] - v[2];
        const vec3 v3 = v[7] - v[3];

        // Draw the far plane of the clip space, offset by one unit of
        // depth buffer distance.  Pass the world-space vectors from the
        // view position toward the screen corners for use in sky display.

        glPolygonOffset(0.0, -1.0);

        glBegin(GL_QUADS);
        {
            glTexCoord2d(0, 0);
            glVertex3d(v0[0], v0[1], v0[2]);
            glTexCoord2d(1, 0);
            glVertex3d(v1[0], v1[1], v1[2]);
            glTexCoord2d(1, 1);
            glVertex3d(v3[0], v3[1], v3[2]);
            glTexCoord2d(0, 1);
            glVertex3d(v2[0], v2[1], v2[2]);
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
