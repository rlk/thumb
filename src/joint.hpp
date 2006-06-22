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

#ifndef JOINT_HPP
#define JOINT_HPP

#include "conf.hpp"
#include "param.hpp"
#include "entity.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class joint : public entity
    {
    protected:

        dJointID join;
        float    size;

    public:

        joint(int=-1);

        void geom_to_entity();

        joint *get_joint() { return this; }

        int link() const { return 0; }

        virtual void edit_init();
        virtual void play_init(dBodyID);
        virtual void play_fini();

        virtual int  step_prio() { return 1; }
        virtual void step_prep();

        virtual void draw_geom() const;
        virtual void draw_fill() const;

        virtual ~joint() { }
    };
}

//-----------------------------------------------------------------------------

namespace ent
{
    class ball : public joint
    {
    public:
        ball();
        ball *clone() const { return new ball(*this); }

        void use_param() { }
        void play_init(dBodyID);
        void play_join(dBodyID);

        mxml_node_t *save(mxml_node_t *);
    };

    class hinge : public joint
    {
    public:
        hinge *clone() const { return new hinge(*this); }
        hinge();

        void use_param();
        void play_init(dBodyID);
        void play_join(dBodyID);

        void step_prep();

        mxml_node_t *save(mxml_node_t *);
    };

    class hinge2 : public joint
    {
    public:
        hinge2 *clone() const { return new hinge2(*this); }
        hinge2();

        void use_param();
        void play_init(dBodyID);
        void play_join(dBodyID);

        void step_prep();

        mxml_node_t *save(mxml_node_t *);
    };

    class slider : public joint
    {
    public:
        slider *clone() const { return new slider(*this); }
        slider();

        void use_param();
        void play_init(dBodyID);
        void play_join(dBodyID);

        void step_prep();

        mxml_node_t *save(mxml_node_t *);
    };

    class amotor : public joint
    {
    public:
        amotor *clone() const { return new amotor(*this); }
        amotor();

        void use_param();
        void play_init(dBodyID);
        void play_join(dBodyID);

        void step_prep();

        mxml_node_t *save(mxml_node_t *);
    };

    class universal : public joint
    {
    public:
        universal *clone() const { return new universal(*this); }
        universal();

        void use_param();
        void play_init(dBodyID);
        void play_join(dBodyID);

        void step_prep();

        mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
