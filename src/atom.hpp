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

#include <ode/ode.h>
#include <mxml.h>

#include "surface.hpp"
#include "param.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class world;

    class atom
    {
    protected:

        dGeomID geom;

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

        void set_default();
        void get_default();
        void get_surface(dSurfaceParameters&);

        void mult_world(const float[16]);
        void mult_local(const float[16]);

        void get_world(float[16]) const;
        void get_local(float[16]) const;

        // ODE physical parameter methods

        void set_param(int, std::string&);
        bool get_param(int, std::string&);

        // Physics methods

        virtual void play_init(dBodyID) { }
        virtual void play_fini()        { }
        virtual void step_init()        { }
        virtual void step_fini()        { }

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
