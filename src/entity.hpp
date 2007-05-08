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

#include "opengl.hpp"
#include "geodata.hpp"
#include "param.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    // Entity base class.

    class entity
    {
        static void phys_pointer(float *, dGeomID, dGeomID);
        static void phys_contact(float *, dGeomID, dGeomID);

    public:

        static dBodyID phys_body();
        static void    phys_init();
        static void    phys_fini();
        static void    phys_step(float);
        static void    phys_pick(const float[3], const float[3]);

        static entity *focused();

    protected:

        // Shared ODE world state.

        static dWorldID      world;
        static dSpaceID      space;
        static dSpaceID      actor;
        static dJointGroupID joint;
        static dGeomID       point;
        static dGeomID       focus;

    protected:

        // Local entity state.

        dGeomID geom;
        int     body1;
        int     body2;
        int     celli;
        float   radius;

        float default_M[16];
        float current_M[16];

        std::map<int, param *> params;

        const ogl::geodata *geometry;
        const ogl::geodata *wireframe;

        virtual void draw_geom() const { }

        // Transform handlers.

        virtual void mult_M() const;
        virtual void mult_R() const;
        virtual void mult_T() const;
        virtual void mult_V() const;
        virtual void mult_P() const;

        void get_transform(float[16], dGeomID);
        void set_transform(float[16], dGeomID);

    public:

        entity(const ogl::geodata *,
               const ogl::geodata *);
        entity(const entity&);

        virtual entity *clone() const = 0;

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

        void  get_world(float[16]) const;
        void  get_local(float[16]) const;
        float get_bound(float[ 3]) const;

        // Manage body and joint and cell associations.

        virtual int link() const { return (body1    ); }
        virtual int body(int i)  { return (body1 = i); }
        virtual int body() const { return (body1    ); }
        virtual int join(int i)  { return (body2 = i); }
        virtual int join() const { return (body2    ); }
        virtual int cell(int i)  { return (celli = i); }
        virtual int cell() const { return (celli    ); }

        // Manage ODE geometry and body state.

        virtual void edit_init() = 0;
        virtual void edit_fini();

        virtual void play_init(dBodyID) { }
        virtual void play_tran(dBodyID) { }
        virtual void play_join(dBodyID) { }
        virtual void play_fini()        { }

        // Manage ODE physical parameters.

        void set_param(int, std::string&);
        bool get_param(int, std::string&);

        // Physics pass.

        virtual void step_prep() { }
        virtual void step_post() { }

        // Render pass.

        virtual void draw_line();
        virtual void draw_foci();

        virtual void draw_init()  { }
        virtual void draw_fini()  { }
        virtual void prep_init()  { }
        virtual void prep_fini()  { }

        virtual void draw(int);
        virtual int  type(   ) { return geometry ? geometry->type() : 0; }

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
