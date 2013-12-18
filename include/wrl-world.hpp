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

#ifndef WRL_WORLD_HPP
#define WRL_WORLD_HPP

#include <etc-vector.hpp>
#include <etc-ode.hpp>
#include <ogl-aabb.hpp>
#include <wrl-atom.hpp>
#include <wrl-operation.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class frustum;
}

namespace ogl
{
    class unit;
    class node;
    class pool;
    class binding;
    class uniform;
    class process;
}

//-----------------------------------------------------------------------------

namespace wrl
{
    typedef std::map<int, dBodyID>     body_map;
    typedef std::map<int, dMass>       mass_map;
    typedef std::map<int, ogl::node *> node_map;

    class world
    {
    public:

        world();
       ~world();

        // Physics methods

        void edit_callback(dGeomID, dGeomID);
        void play_callback(dGeomID, dGeomID);

        void play_init();
        void play_fini();

        void edit_pick(const vec3&, const vec3&);
        void edit_step(double);
        void play_step(double);

        dSpaceID get_space() const { return edit_space; }
        dGeomID  get_focus() const { return edit_focus; }

        void set_param(int, std::string&);
        int  get_param(int, std::string&);

        // Editing methods

        void click_selection(atom *);
        void clone_selection();
        void clear_selection();
        bool check_selection();

        void invert_selection();
        void extend_selection();

        void select_set();
        void select_set(atom_set&);
        void create_set(atom_set&);
        void delete_set(atom_set&);
        void embody_set(atom_set&, atom_map&);
        void modify_set(atom_set&, const mat4&);

        // Undo-able / redo-able operations

        void do_create();
        void do_delete();
        void do_enjoin();
        void do_embody();
        void do_debody();
        void do_modify(const mat4&);

        void undo();
        void redo();

        // File I/O

        void init();
        void load(std::string);
        void save(std::string, bool);

        // Rendering methods

        void shadow(int, const app::frustum *, int);
        int s_light(int, const app::frustum *const *, int, const ogl::aabb&, const atom *);
        int d_light(int, const app::frustum *const *, int, const ogl::aabb&, const atom *);

        ogl::aabb prep_fill(int, const app::frustum *const *);
        ogl::aabb prep_line(int, const app::frustum *const *);

        void           lite(int, const app::frustum *const *);

        void      draw_fill(int, const app::frustum *);
        void      draw_line(int, const app::frustum *);

    private:

        ogl::aabb fill_bound;
        ogl::aabb lite_bound[3];

        // ODE edit state

        double        focus_dist;
        dSpaceID      edit_space;
        dGeomID       edit_point;
        dGeomID       edit_focus;

        // ODE play state

        dWorldID      play_world;
        dSpaceID      play_scene;
        dSpaceID      play_actor;
        dJointGroupID play_joint;

        body_map play_body;

        // World state

        atom_set all;
        atom_set sel;

        int shadow_res;

        // Batcher state

        node_map nodes;

        ogl::pool *fill_pool;
        ogl::node *fill_node;

        ogl::pool *line_pool;
        ogl::node *line_node;

        const ogl::binding *sky;
        const ogl::binding *sky_light;
        const ogl::binding *sky_shade;

        void node_insert(int, ogl::unit *, ogl::unit *);
        void node_remove(int, ogl::unit *, ogl::unit *);

        // Operations handlers

        int serial;

        wrl::operation_l undo_list;
        wrl::operation_l redo_list;

        void doop(wrl::operation_p);

        // ...

        ogl::uniform *uniform_light_position;
        ogl::uniform *uniform_shadow[4];

        const ogl::process *process_shadow[4];

        void draw_sky(const app::frustum *);

        void draw_debug_wireframe(int);
    };
}

//-----------------------------------------------------------------------------

#endif
