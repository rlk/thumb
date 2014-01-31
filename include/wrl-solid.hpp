//  Copyright (C) 2005-2011 Robert Kooima
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
#include <ogl-convex.hpp>

//-----------------------------------------------------------------------------

namespace wrl
{
    class solid : public atom
    {
    protected:

        dGeomID play_geom;

    public:

        solid(app::node, std::string, std::string, bool=true);

        // Physics initialization methods

        virtual dGeomID get_geom(dSpaceID);

        // Physics update methods

        virtual void play_init();
        virtual void play_fini();

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid box atom

    class box : public solid
    {
    protected:

        virtual dGeomID new_geom(dSpaceID) const;

    public:

        box(app::node);
        box(std::string, bool=true);

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

        virtual dGeomID new_geom(dSpaceID) const;

    public:

        sphere(app::node);
        sphere(std::string, bool=true);

        virtual sphere *clone() const { return new sphere(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Convex solid atom

    class convex : public solid
    {
    private:

    protected:

        ogl::convex *data;

        virtual dGeomID new_geom(dSpaceID) const;

    public:

        convex(const convex&);
        convex(app::node);
        convex(std::string, bool=true);

        virtual ~convex();

        virtual convex *clone() const { return new convex(*this); }

        // Physics initialization methods

        virtual void get_mass(dMass *m);

        // File I/O

        virtual void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
