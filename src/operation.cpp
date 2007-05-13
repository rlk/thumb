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

#include "operation.hpp"
#include "matrix.hpp"
#include "world.hpp"

//-----------------------------------------------------------------------------

// This atom set remains empty at all times. It is used as a return value
// when an undo or redo should cause the selection to be cleared.

namespace ops
{
    static wrl::atom_set deselection;
}

//-----------------------------------------------------------------------------

ops::create_op::create_op(wrl::atom_set& S) : operation(S)
{
}

ops::create_op::~create_op()
{
    // An undone create operation contains the only reference to the created
    // atoms. If this operation is deleted before being redone then these
    // atoms become abandoned and should be deleted.

    wrl::atom_set::iterator i;

    if (done == false)
        for (i = selection.begin(); i != selection.end(); ++i)
            delete (*i);
}

wrl::atom_set& ops::create_op::undo(wrl::world *w)
{
    w->delete_set(selection);
    done = false;
    return deselection;
}

wrl::atom_set& ops::create_op::redo(wrl::world *w)
{
    w->create_set(selection);
    done = true;
    return selection;
}

//-----------------------------------------------------------------------------

ops::delete_op::delete_op(wrl::atom_set& S) : operation(S)
{
}

ops::delete_op::~delete_op()
{
    // A non-undone delete operation contains the only reference to the deleted
    // atom. If this operation is deleted before being undone then these atoms
    // become abandoned and should be deleted.

    wrl::atom_set::iterator i;

    if (done == true)
        for (i = selection.begin(); i != selection.end(); ++i)
            delete (*i);
}

wrl::atom_set& ops::delete_op::undo(wrl::world *w)
{
    w->create_set(selection);
    done = false;
    return selection;
}

wrl::atom_set& ops::delete_op::redo(wrl::world *w)
{
    w->delete_set(selection);
    done = true;
    return deselection;
}

//-----------------------------------------------------------------------------

ops::modify_op::modify_op(wrl::atom_set& S, const float M[16]) : operation(S)
{
    load_mat(T, M);
    load_inv(I, M);
}

wrl::atom_set& ops::modify_op::undo(wrl::world *w)
{
    w->modify_set(selection, I);
    done = false;
    return selection;
}

wrl::atom_set& ops::modify_op::redo(wrl::world *w)
{
    w->modify_set(selection, T);
    done = true;
    return selection;
}

//-----------------------------------------------------------------------------
