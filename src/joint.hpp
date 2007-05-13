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

#include "param.hpp"
#include "atom.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class joint : public atom
    {
    protected:

        dJointID join;

    public:

        joint(dSpaceID, const ogl::surface *,
                        const ogl::surface *);

        virtual void play_init(dBodyID);
        virtual void play_fini();
        virtual void step_init();
        virtual void draw_line() const;

        virtual ~joint();
    };

    //-------------------------------------------------------------------------
    // Ball joint

    class ball : public joint
    {
    public:

        ball(dWorldID, dSpaceID);

        ball *clone() const { return new ball(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Hinge joint

    class hinge : public joint
    {
    public:

        hinge(dWorldID, dSpaceID);

        hinge *clone() const { return new hinge(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);
        void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Suspension hinge joint

    class hinge2 : public joint
    {
    public:
        hinge2(dWorldID, dSpaceID);

        hinge2 *clone() const { return new hinge2(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);
        void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Prismatic slider joint

    class slider : public joint
    {
    public:
        slider(dWorldID, dSpaceID);

        slider *clone() const { return new slider(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);
        void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Angular motor joint

    class amotor : public joint
    {
    public:
        amotor(dWorldID, dSpaceID);

        amotor *clone() const { return new amotor(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);
        void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Universal joint

    class universal : public joint
    {
    public:
        universal(dWorldID, dSpaceID);

        universal *clone() const { return new universal(*this); }

//      void play_init(dBodyID);
//      void play_join(dBodyID);
        void step_init();

        mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
