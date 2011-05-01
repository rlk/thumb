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

#ifndef WRL_JOINT_HPP
#define WRL_JOINT_HPP

#include <wrl-param.hpp>
#include <wrl-atom.hpp>

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

        virtual void play_init();
        virtual void play_fini();
        virtual void step_init();

        // File I/O

        virtual void load(app::node);
        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Ball joint

    class ball : public joint
    {
    public:

        ball();

        ball *clone() const { return new ball(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();

        void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Hinge joint

    class hinge : public joint
    {
    public:

        hinge();

        hinge *clone() const { return new hinge(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();
        virtual void step_init();

        void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Suspension hinge joint

    class hinge2 : public joint
    {
    public:
        hinge2();

        hinge2 *clone() const { return new hinge2(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();
        virtual void step_init();

        void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Prismatic slider joint

    class slider : public joint
    {
    public:
        slider();

        slider *clone() const { return new slider(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();
        virtual void step_init();

        void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Angular motor joint

    class amotor : public joint
    {
    public:
        amotor();

        amotor *clone() const { return new amotor(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();
        virtual void step_init();

        void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Universal joint

    class universal : public joint
    {
    public:
        universal();

        universal *clone() const { return new universal(*this); }

        virtual dJointID get_join(dWorldID);

        virtual void play_init();
        virtual void step_init();

        void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
