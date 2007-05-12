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

#include <set>

#include <ode/ode.h>

#include "surface.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    //-------------------------------------------------------------------------

    class atom
    {
    protected:

        dGeomID geom;

        float default_M[16];
        float current_M[16];

//      std::map<int, param *> params;

        const ogl::surface *fill;
        const ogl::surface *line;

        // Transform handlers.

        virtual void mult_M() const;
        virtual void mult_R() const;
        virtual void mult_T() const;
        virtual void mult_V() const;
        virtual void mult_P() const;

        void get_transform(float[16], dGeomID);
        void set_transform(float[16], dGeomID);

    public:

        atom(const ogl::surface *, const ogl::surface *);
//      atom(const atom&);

//      virtual atom *clone() const = 0;

        // Store and recall default transform state.

        void set_default();
        void get_default();
        void get_surface(dSurfaceParameters&);

        // Transform.

        void turn_world(float, float, float, float, float, float, float);
        void turn_world(float, float, float, float);
        void move_world(float, float, float);
        void mult_world(const float[16]);

        void turn_local(float, float, float, float, float, float, float);
        void turn_local(float, float, float, float);
        void move_local(float, float, float);
        void mult_local(const float[16]);

        void get_world(float[16]) const;
        void get_local(float[16]) const;

        void draw_fill() const;
        void draw_line() const;
        void draw_foci() const;

        virtual ~atom();
    };

    //-------------------------------------------------------------------------

    class unit
    {
        dBodyID body;
    };

    //-------------------------------------------------------------------------

    typedef std::set<atom *> atom_set;
    typedef std::set<unit *> unit_set;

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

    public:

        world();
       ~world();

        // ODE methods

        void phys_pointer(dGeomID, dGeomID);
        void phys_contact(dGeomID, dGeomID);

        void phys_init();
        void phys_fini();
        void phys_step(float);
        void phys_pick(const float[3], const float[3]);

        atom *focused() const;

        void draw_scene() const;
        void draw_gizmo() const;
    };
}

//-----------------------------------------------------------------------------

#endif
