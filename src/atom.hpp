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

#ifndef ATOM_HPP
#define ATOM_HPP

#include <set>
#include <map>

#include <ode/ode.h>
#include <mxml.h>

#include "surface.hpp"
#include "matrix.hpp"
#include "param.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class world;

    class atom
    {
    protected:

        dGeomID edit_geom;

        int body_id;

        const ogl::surface *fill;
        const ogl::surface *line;

        param_map params;

        // Transform handlers

        float default_M[16];
        float current_M[16];

        virtual void mult_M() const;
        virtual void mult_R() const;
        virtual void mult_T() const;
        virtual void mult_V() const;
        virtual void mult_P() const;

        void get_transform(float[16]);
        void set_transform(float[16]);

    public:

        atom(const ogl::surface *, const ogl::surface *);
        atom(const atom&);

        virtual atom *clone() const = 0;

        // Transform methods

        void get_surface(dSurfaceParameters&);

        void transform(const float[16]);
        void get_world(float[16]) const;
        void get_local(float[16]) const;

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

        virtual void play_init() { }
        virtual void play_fini() { load_mat(current_M, default_M); }
        virtual void step_init() { }
        virtual void step_fini() { }

        // Rendering methods

        virtual void draw_foci(dGeomID) const;
        virtual void draw_stat()        const;
        virtual void draw_line()        const;
        virtual void draw_fill()        const;

        // File I/O

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);

        virtual ~atom();
    };

    typedef std::set<atom *> atom_set;
}

//-----------------------------------------------------------------------------

#endif
