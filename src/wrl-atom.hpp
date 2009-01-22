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

#ifndef WRL_ATOM_HPP
#define WRL_ATOM_HPP

#include <set>
#include <map>

#include <ode/ode.h>
#include <mxml.h>

#include "matrix.hpp"
#include "wrl-param.hpp"
#include "ogl-pool.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class world;

    class atom
    {
    protected:

        dGeomID edit_geom;

        int body_id;

        std::string name;
        ogl::unit  *fill;
        ogl::unit  *line;

        double line_scale[3];

        param_map params;

        // Transform handlers

        double default_M[16];
        double current_M[16];

        virtual void mult_M() const;
        virtual void mult_R() const;
        virtual void mult_T() const;
        virtual void mult_V() const;
        virtual void mult_P() const;

    public:

        atom(std::string, std::string);
        atom(const atom&);

        virtual atom *clone() const = 0;

        void live(dSpaceID) const;
        void dead(dSpaceID) const;

        // Transform methods

        void get_surface(dSurfaceParameters&);

        void get_world(double *) const;
        void get_local(double *) const;
        void transform(const double *);

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

        // File I/O

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);

        virtual ~atom();
    };

    typedef std::set<atom *     > atom_set;
    typedef std::map<atom *, int> atom_map;
}

//-----------------------------------------------------------------------------

#endif
