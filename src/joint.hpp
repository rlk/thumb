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

        int      join_id;
        dJointID play_join;

    public:

        joint(std::string, std::string);

        // Joint binding

        virtual int join(int id) { return (join_id = id); }
        virtual int join() const { return (join_id     ); }

        // Physics update methods

        virtual void step_init();

        // File I/O

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Ball joint

    class ball : public joint
    {
    public:

        ball();

        ball *clone() const { return new ball(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Hinge joint

    class hinge : public joint
    {
    public:

        hinge();

        hinge *clone() const { return new hinge(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();
        virtual void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Suspension hinge joint

    class hinge2 : public joint
    {
    public:
        hinge2();

        hinge2 *clone() const { return new hinge2(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();
        virtual void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Prismatic slider joint

    class slider : public joint
    {
    public:
        slider();

        slider *clone() const { return new slider(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();
        virtual void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Angular motor joint

    class amotor : public joint
    {
    public:
        amotor();

        amotor *clone() const { return new amotor(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();
        virtual void step_init();

        mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Universal joint

    class universal : public joint
    {
    public:
        universal();

        universal *clone() const { return new universal(*this); }

        virtual dJointID get_join(dWorldID);

//      virtual void play_init(ogl::segment *);
        virtual void play_init();
        virtual void step_init();

        mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
