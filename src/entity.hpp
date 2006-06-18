//  Copyright (C) 2005 Robert Kooima
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

#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <set>
#include <list>
#include <map>

#include <ode/ode.h>
#include <mxml.h>

#include "obj.h"

//-----------------------------------------------------------------------------

namespace ent
{
    class joint;
    class solid;

    class entity
    {
        static void phys_pick(float *, dGeomID, dGeomID);
        static void phys_cont(float *, dGeomID, dGeomID);

    public:

        static dBodyID phys_body();
        static void    phys_init();
        static void    phys_step(float);

        static entity *focused();

    protected:

        // Shared ODE world state.

        static dWorldID      world;
        static dSpaceID      space;
        static dJointGroupID joint;
        static dGeomID       point;
        static dGeomID       focus;

    protected:

        // Local entity state.

        dGeomID geom;
        int     body1;
        int     body2;
        int     file;
        float   radius;

        float default_M[16];
        float current_M[16];

        void get_transform(float[16], dGeomID);
        void set_transform(float[16], dGeomID);

    public:

        entity(int=-1);

        virtual entity *clone() const = 0;

        virtual void geom_to_entity() { }
        virtual void entity_to_geom();

        virtual ent::solid *get_solid() { return 0; }
        virtual ent::joint *get_joint() { return 0; }

        float get_sphere(float[3]);

        // Store and recall default transform state.

        void set_default();
        void get_default();

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

        // Manage body and joint associations.

        virtual int  link() const { return body1; }
        virtual void body(int i)  { body1 = i;    }
        virtual int  body() const { return body1; }
        virtual void join(int i)  { body2 = i;    }
        virtual int  join() const { return body2; }

        // Manage ODE geometry and body state.

        virtual void edit_init() = 0;
        virtual void edit_fini();

        virtual void play_init(dBodyID) { }
        virtual void play_tran(dBodyID) { }
        virtual void play_join(dBodyID) { }
        virtual void play_fini()        { }

        // Manage ODE physical parameters.

        virtual void set_param(int, std::string&) = 0;
        virtual bool get_param(int, std::string&) = 0;

        // Apply transforms.

        void mult_M() const;
        void mult_R() const;
        void mult_T() const;
        void mult_V() const;

        // Render

        virtual void draw_geom() const { }
        virtual void draw_fill() const;
        virtual void draw_line() const;
        virtual void draw_foci() const;

        // File I/O

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);

        virtual ~entity();
    };

    typedef std::set <entity *> set;
    typedef std::list<entity *> list;
}

//-----------------------------------------------------------------------------

#endif
