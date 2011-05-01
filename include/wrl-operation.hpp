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

#ifndef OPERATION_HPP
#define OPERATION_HPP

#include <list>

#include <wrl-atom.hpp>

//-----------------------------------------------------------------------------

namespace wrl
{
    //-------------------------------------------------------------------------
    // Undo-able / redo-able operation interface.

    class operation
    {
    protected:

        wrl::atom_set selection;

        bool done;

    public:

        operation(wrl::atom_set& s) : selection(s), done(false) { }

        virtual wrl::atom_set& undo(wrl::world *) = 0;
        virtual wrl::atom_set& redo(wrl::world *) = 0;

        virtual ~operation() { }
    };

    typedef operation                       *operation_p;
    typedef std::list<operation_p>           operation_l;
    typedef std::list<operation_p>::iterator operation_i;

    //-------------------------------------------------------------------------
    // Object creation operation.

    class create_op : public operation
    {
    public:

        create_op(wrl::atom_set&);
       ~create_op();

        wrl::atom_set& undo(wrl::world *);
        wrl::atom_set& redo(wrl::world *);
    };

    //-------------------------------------------------------------------------
    // Object deletion operation.

    class delete_op : public operation
    {
    public:

        delete_op(wrl::atom_set&);
       ~delete_op();

        wrl::atom_set& undo(wrl::world *);
        wrl::atom_set& redo(wrl::world *);
    };

    //-------------------------------------------------------------------------
    // Object transform modification operation.

    class modify_op : public operation
    {
        double T[16];
        double I[16];

    public:

        modify_op(wrl::atom_set&, const double *);

        wrl::atom_set& undo(wrl::world *);
        wrl::atom_set& redo(wrl::world *);
    };

    //-------------------------------------------------------------------------
    // Body creation operation.

    class embody_op : public operation
    {
        wrl::atom_map old_id;
        wrl::atom_map new_id;

    public:

        embody_op(wrl::atom_set&, int);

        wrl::atom_set& undo(wrl::world *);
        wrl::atom_set& redo(wrl::world *);
    };

    //-------------------------------------------------------------------------
    // Joint attachment operation.

    class enjoin_op : public operation
    {
        wrl::atom_map old_id;
        int           one_id;
        int           two_id;

    public:

        enjoin_op(wrl::atom_set&);

        wrl::atom_set& undo(wrl::world *);
        wrl::atom_set& redo(wrl::world *);
    };
}

//-----------------------------------------------------------------------------

#endif
