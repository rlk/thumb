#ifndef SOLID_HPP
#define SOLID_HPP

#include "param.hpp"
#include "entity.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class solid : public entity
    {
    public:

        enum {
            friction = dParamGroup * 4 + 1,
            density  = dParamGroup * 4 + 2,
            bounce   = dParamGroup * 4 + 3,
            soft_erp = dParamGroup * 4 + 4,
            soft_cfm = dParamGroup * 4 + 5
        };

    protected:

        dGeomID tran;
        dMass   mass;

        std::map<int, solid_param *> params;

    public:

        solid(int f=-1);
        solid(const solid&);

        void geom_to_entity();

        solid *get_solid() { return this; }

        void use_param(dSurfaceParameters&);
        void set_param(int, std::string&);
        bool get_param(int, std::string&);

        virtual void join(int)    {           }
        virtual int  join() const { return 0; }

        virtual void play_init(dBodyID);
        virtual void play_tran(dBodyID);
        virtual void play_fini();

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);

        virtual ~solid();
    };
}

//-----------------------------------------------------------------------------

namespace ent
{
    class free : public solid
    {
    public:
        free *clone() const { return new free(*this); }
        free(int f=-1)    : solid(f) { }

        void edit_init() { }
    };

    class box : public solid
    {
    public:
        box *clone() const { return new box(*this); }
        box(int f=-1)     : solid(f) { }

        void edit_init();
        void play_init(dBodyID);
        void draw_geom() const;

        mxml_node_t *save(mxml_node_t *);
    };

    class sphere : public solid
    {
    public:
        sphere *clone() const { return new sphere(*this); }
        sphere(int f=-1)  : solid(f) { }

        void edit_init();
        void play_init(dBodyID);
        void draw_geom() const;

        mxml_node_t *save(mxml_node_t *);
    };

    class capsule : public solid
    {
    public:
        capsule *clone() const { return new capsule(*this); }
        capsule(int f=-1) : solid(f) { }

        void edit_init();
        void play_init(dBodyID);
        void draw_geom() const;

        mxml_node_t *save(mxml_node_t *);
    };

    class plane : public solid
    {
    public:
        plane *clone() const { return new plane(*this); }
        plane(int f=-1)   : solid(f) { }

        void edit_init();
    };
}

//-----------------------------------------------------------------------------

#endif
