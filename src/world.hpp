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

#ifndef WORLD_HPP
#define WORLD_HPP

#include <ode/ode.h>

#include "atom.hpp"
#include "operation.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class world
    {
        // ODE edit state

        dSpaceID      edit_space;
        dGeomID       edit_point;
        dGeomID       edit_focus;

        float focus_distance;

        // ODE play state

        dWorldID      play_world;
        dSpaceID      play_scene;
        dSpaceID      play_actor;
        dJointGroupID play_joint;

        // World state

        atom_set all;
        atom_set sel;

        int serial;

        ops::operation_l undo_list;
        ops::operation_l redo_list;

        void doop(ops::operation_p);

    public:

        world();
       ~world();

        // Physics methods

        void edit_callback(dGeomID, dGeomID);
        void play_callback(dGeomID, dGeomID);

        void play_init();
        void play_fini();

        void edit_pick(const float[3], const float[3]);
        void edit_step(float);
        void play_step(float);

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

        void create_set(atom_set&);
        void delete_set(atom_set&);
        void modify_set(atom_set&, const float[16]);

        // Undo-able / redo-able operation.

        void do_create();
        void do_delete();
        void do_enjoin();
        void do_embody();
        void do_debody();
        void do_modify(const float[16]);

        void undo();
        void redo();

        // File I/O.

        void init();
        void load(std::string);
        void save(std::string, bool);

        // Rendering methods

        void draw_scene() const;
        void draw_gizmo() const;
    };
}

//-----------------------------------------------------------------------------

#endif
