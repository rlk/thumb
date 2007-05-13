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
        // ODE state

        dWorldID      state;
        dSpaceID      scene;
        dSpaceID      actor;
        dJointGroupID joint;
        dGeomID       point;
        dGeomID       focus;

        float focus_distance;

        // World state

        atom_set all;
        atom_set sel;

        ops::operation_l undo_list;
        ops::operation_l redo_list;

        void doop(ops::operation_p);

    public:

        world();
       ~world();

        // Physics methods

        void phys_pointer(dGeomID, dGeomID);
        void phys_contact(dGeomID, dGeomID);

        void pick(const float[3], const float[3]);
        void step(float);

        dWorldID get_world() const { return state; }
        dSpaceID get_space() const { return scene; }
        dGeomID  get_focus() const { return focus; }

        // Editing methods

        void click_selection(atom *);
        void clone_selection();
        void clear_selection();

        void invert_selection();
        void extend_selection();

        void create_set(atom_set&);
        void delete_set(atom_set&);
        void modify_set(atom_set&, const float[16]);

        // Undo-able / redo-able operation.

        void do_create();
        void do_delete();
        void do_modify(const float[16]);

        void undo();
        void redo();

        // Rendering methods

        void draw_scene() const;
        void draw_gizmo() const;
    };
}

//-----------------------------------------------------------------------------

#endif
