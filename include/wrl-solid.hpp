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
    public:

        solid(app::node=0, std::string="", std::string="");

        // Physics initialization methods

        virtual dGeomID  new_play_geom(dSpaceID) const;
        virtual dGeomID init_play_geom(dSpaceID);
        virtual void    init_play_mass(dMass  *);

        // Physics update methods

        virtual void play_init();
        virtual void play_fini();

        // File I/O

        virtual void save(app::node);

    protected:

        dGeomID play_geom;
        dMass   play_mass;
    };

    //-------------------------------------------------------------------------
    // Solid box atom

    class box : public solid
    {
    protected:

        vec3 length;

    public:

        box(app::node=0, std::string="");

        virtual box *clone() const { return new box(*this); }

        // Physics initialization methods

        virtual dGeomID new_edit_geom(dSpaceID) const;
        virtual void    new_play_mass(dMass  *);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid sphere atom

    class sphere : public solid
    {
    protected:

        double radius;

    public:

        sphere(app::node=0, std::string="");

        virtual sphere *clone() const { return new sphere(*this); }

        // Physics initialization methods

        virtual dGeomID new_edit_geom(dSpaceID) const;
        virtual void    new_play_mass(dMass  *);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid capsule atom

    class capsule : public solid
    {
    protected:

        double radius;
        double length;

    public:

        capsule(app::node=0, std::string="");

        virtual capsule *clone() const { return new capsule(*this); }

        // Physics initialization methods

        virtual dGeomID new_edit_geom(dSpaceID) const;
        virtual void    new_play_mass(dMass  *);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Solid cylinder atom

    class cylinder : public solid
    {
    protected:

        double radius;
        double length;

    public:

        cylinder(app::node=0, std::string="");

        virtual cylinder *clone() const { return new cylinder(*this); }

        // Physics initialization methods

        virtual dGeomID new_edit_geom(dSpaceID) const;
        virtual void    new_play_mass(dMass  *);

        // File I/O

        virtual void save(app::node);
    };

    //-------------------------------------------------------------------------
    // Convex solid atom

    class convex : public solid
    {
    protected:

        ogl::convex *data;
        dTriMeshDataID id;

    public:

        convex(app::node=0, std::string="");
        convex(const convex&);

        virtual ~convex();

        virtual convex *clone() const { return new convex(*this); }

        // Physics initialization methods

        virtual dGeomID new_edit_geom(dSpaceID) const;
        virtual dGeomID new_play_geom(dSpaceID) const;
        virtual void    new_play_mass(dMass  *);

        // File I/O

        virtual void save(app::node);
    };
}

//-----------------------------------------------------------------------------

#endif
