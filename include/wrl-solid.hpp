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

#ifndef WRL_SOLID_HPP
#define WRL_SOLID_HPP

#include <app-file.hpp>
#include <wrl-param.hpp>
#include <wrl-atom.hpp>

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
        virtual void play_fini();

        // File I/O

        virtual void load(app::node);
        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid box atom

    class box : public solid
    {
    protected:

        virtual void scale();

    public:

        box(std::string);

        virtual box *clone() const { return new box(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid sphere atom

    class sphere : public solid
    {
    protected:

        virtual void scale();

    public:

        sphere(std::string);

        virtual sphere *clone() const { return new sphere(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // File I/O

        virtual void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
