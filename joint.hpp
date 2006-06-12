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
    public:

        enum {
            velocity  =  0,
            velocity2 =  1,
            velocity3 =  2,
            force     =  3,
            force2    =  4,
            force3    =  5,
            cfm       =  6,
            cfm2      =  7,
            cfm3      =  8,
            bounce    =  9,
            bounce2   = 10,
            bounce3   = 11,
            lo_stop   = 12,
            lo_stop2  = 13,
            lo_stop3  = 14,
            hi_stop   = 15,
            hi_stop2  = 16,
            hi_stop3  = 17,
            stop_erp  = 18,
            stop_erp2 = 19,
            stop_erp3 = 20,
            stop_cfm  = 21,
            stop_cfm2 = 22,
            stop_cfm3 = 23,
            susp_erp  = 24,
            susp_cfm  = 25
        };
        
    protected:

        dJointID join;
        float    size;

        std::map<int, joint_param *> params;

    public:

        joint(int=-1);
        joint(const joint&);

        void geom_to_entity();

        joint *get_joint() { return this; }

        virtual void use_param() { }
        void set_param(int, std::string&);
        bool get_param(int, std::string&);

        int link() const { return 0; }

        virtual void edit_init();
        virtual void play_init(dBodyID);
        virtual void play_fini();

        virtual void draw_geom() const;
        virtual void draw_fill() const;

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);

        virtual ~joint();
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

        mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
