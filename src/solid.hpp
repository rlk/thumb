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

#ifndef SOLID_HPP
#define SOLID_HPP

#include "param.hpp"
#include "atom.hpp"

//-----------------------------------------------------------------------------

namespace wrl
{
    class solid : public atom
    {
    protected:

        virtual void scale() = 0;

        dGeomID play_geom;

    public:

        solid(std::string, std::string);

        // Physics initialization methods

        virtual dGeomID get_geom(dSpaceID);

        // Physics update methods

        virtual void play_init();
        virtual void step_fini();

        // File I/O

        virtual void         load(mxml_node_t *);
        virtual mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Solid box atom

    class box : public solid
    {
    protected:

        virtual void scale();

    public:

        box(dSpaceID, std::string);

        virtual box *clone() const { return new box(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // Rendering

        virtual void draw_line() const;

        // File I/O

        virtual mxml_node_t *save(mxml_node_t *);
    };

    //-------------------------------------------------------------------------
    // Solid sphere atom

    class sphere : public solid
    {
    protected:

        virtual void scale();

    public:

        sphere(dSpaceID, std::string);

        virtual sphere *clone() const { return new sphere(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // Rendering

        virtual void draw_line() const;

        // File I/O

        virtual mxml_node_t *save(mxml_node_t *);
    };
}

//-----------------------------------------------------------------------------

#endif
