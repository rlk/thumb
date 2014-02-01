//  Copyright (C) 2007-2011 Robert Kooima
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

#ifndef WRL_ATOM_HPP
#define WRL_ATOM_HPP

#include <set>
#include <map>

#include <etc-vector.hpp>
#include <etc-ode.hpp>
#include <ogl-aabb.hpp>
#include <app-file.hpp>
#include <wrl-param.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class unit;
}

namespace wrl
{
    class world;
}

//-----------------------------------------------------------------------------

namespace wrl
{
    class atom
    {
    public:

        atom(app::node, std::string, std::string);

        virtual atom *clone() const = 0;

        void live(dSpaceID) const;
        void dead(dSpaceID) const;

        // Transform methods

        mat4 get_world() const;
        mat4 get_local() const;
        void transform(const mat4&);

        ogl::unit *get_fill() { return fill; }
        ogl::unit *get_line() { return line; }

        // Physics parameter accessors

        void set_param(int, std::string&);
        bool get_param(int, std::string&);

        // Body and joint bindings

        virtual int body(int id) { return body_id = id; }
        virtual int body() const { return body_id;      }
        virtual int join(int id) { return 0;            }
        virtual int join() const { return 0;            }

        // Physics initialization methods

        virtual void     get_mass(dMass *m) { dMassSetZero(m); }
        virtual dGeomID  get_geom(dSpaceID) { return 0;        }
        virtual dJointID get_join(dWorldID) { return 0;        }

        // Physics update methods

        virtual void step_init() { }
        virtual void step_fini() { }
        virtual void play_init() { }
        virtual void play_fini() { }

        // Parameter handlers

        virtual void   get_surface(dSurfaceParameters&) const;
        virtual double get_lighting(vec2&)              const;

        // File I/O

        virtual void save(app::node);

        virtual ~atom();

        virtual int priority() const { return 0; }

    protected:

        virtual dGeomID new_geom(dSpaceID) const { return 0; }

        dGeomID edit_geom;

        int body_id;

        // Visual representations.

        std::string fill_name;
        std::string line_name;
        ogl::aabb   fill_bound;
        ogl::aabb   line_bound;
        ogl::unit  *fill;
        ogl::unit  *line;

        // Physical system parameters

        void load_params(app::node);
        void save_params(app::node);
        param_map params;

        // Transform handlers

        mat4 default_M;
        mat4 current_M;
        vec3 line_scale;
    };

    //-------------------------------------------------------------------------

    struct atomcmp
    {
        bool operator()(const atom *a1, const atom *a2) const {
            if      (a1->priority() < a2->priority()) return true;
            else if (a1->priority() > a2->priority()) return false;
            else                                      return (a1 < a2);
        }
    };

    typedef std::set<atom *, atomcmp> atom_set;
    typedef std::map<atom *, int>     atom_map;
}

//-----------------------------------------------------------------------------

#endif
