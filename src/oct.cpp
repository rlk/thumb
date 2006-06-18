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

#include "oct.hpp"

//-----------------------------------------------------------------------------

void ent::oct::node::draw_fill() const
{
    ent::set::const_iterator i;

    for (i = entities.begin(); i != entities.end(); ++i)
        (*i)->draw_fill();
}

void ent::oct::node::draw_foci() const
{
    ent::set::const_iterator i;

    for (i = entities.begin(); i != entities.end(); ++i)
        (*i)->draw_foci();
}

//-----------------------------------------------------------------------------

ent::oct::oct(float range, int depth) :

    range(range),
    depth(depth),

    count(1 << depth * 3)
{
    tree = new node[count];
}

ent::oct::~oct()
{
    // Delete all entities.

    ent::set::iterator i;

    for (i = all.begin(); i != all.end(); ++i)
        delete (*i);

    // Delete the tree structure.

    delete [] tree;
}

//-----------------------------------------------------------------------------

void ent::oct::insert(entity *e)
{
    all.insert(e);

    tree[0].entities.insert(e);
}

void ent::oct::remove(entity *e)
{
    all.erase(all.find(e));

    tree[0].entities.erase(tree[0].entities.find(e));
}

//-----------------------------------------------------------------------------

void ent::oct::draw_fill() const
{
    for (int c = 0; c < count; ++c)
        tree[c].draw_fill();
}

void ent::oct::draw_foci() const
{
    for (int c = 0; c < count; ++c)
        tree[c].draw_foci();
}

//-----------------------------------------------------------------------------

