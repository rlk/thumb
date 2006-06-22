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

ent::oct::oct(float range, int depth) :

    range(range),
    depth(depth),
    count(1)
{
    int c = 1;

    // Compute the size of the octree.

    for (int i = 0; i < depth; ++i)
    {
        c     *= 8;
        count += c;
    }

    // Allocate the tree structure.

    tree = new cell[count];
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

void ent::oct::aabb(float b[6], int c) const
{
    float r = 2 * range;
    float s = 2 * range;

    b[0] = -range;
    b[1] = -range;
    b[2] = -range;

    int p;

    // Determine the size of cell c.

    for (p = c; p > 0; p = up(p))
         r = s = s / 2;

    // Determine the location of cell c.

    for (p = c; p > 0; p = up(p))
    {
        if ((p - 1) & 1) b[0] += s;
        if ((p - 1) & 2) b[1] += s;
        if ((p - 1) & 4) b[2] += s;

        s *= 2;
    }

    // Return a complete bounding box.

    b[3] = b[0] + r;
    b[4] = b[1] + r;
    b[5] = b[2] + r;
}

bool ent::oct::test(entity *e, int c) const
{
    float p[3], r = e->get_bound(p);
    float b[6];

    // Return true if entity e fits entirely within in cell c.

    aabb(b, c);

    return (p[0] <= b[0] + r || b[3] - r < p[0] ||
            p[1] <= b[1] + r || b[4] - r < p[1] ||
            p[2] <= b[2] + r || b[5] - r < p[2]) ? false : true;
}

void ent::oct::move(entity *e, int c)
{
    // Move entity e from its current cell to cell c.

    tree[e->cell( )].entities.erase(tree[e->cell( )].entities.find(e));
    tree[e->cell(c)].entities.insert(e);
}

bool ent::oct::push(entity *e)
{
    // Push entity e to any child that will take it.

    if (dn(e->cell(), 0) < count)
        for (int i = 0; i < 8; ++i)
            if (test(e, dn(e->cell(), i)))
            {
                move(e, dn(e->cell(), i));
                return true;
            }

    // TODO: optimize this to use only one move.

    return false;
}

bool ent::oct::pull(entity *e)
{
    // Pull entity e up to the parent.

    if (e->cell())
    {
        move(e, up(e->cell()));

        return !test(e, e->cell());
    }
    return false;
}

void ent::oct::seek(entity *e)
{
    if (test(e, e->cell()))
    {
        // Push entity e down to the deepest child that will take it.

        while (push(e))
            ;
    }
    else
    {
        // Pull entity e up to the shallowest parent that will take it.

        while (pull(e))
            ;
    }
}

//-----------------------------------------------------------------------------

void ent::oct::insert(entity *e)
{
    tree[e->cell(0)].entities.insert(e);
//  seek(e);

    all.insert(e);
}

void ent::oct::remove(entity *e)
{
    tree[e->cell()].entities.erase(tree[e->cell()].entities.find(e));

    all.erase(all.find(e));
}

void ent::oct::modify(entity *e)
{
//  seek(e);
}

//-----------------------------------------------------------------------------

void ent::oct::draw_fill() const
{
//  for (int c = 0; c < count; ++c)
//      tree[c].draw_fill();

    for (ent::set::const_iterator i = all.begin(); i != all.end(); ++i)
        (*i)->draw_fill();
}

void ent::oct::draw_foci() const
{
 // for (int c = 0; c < count; ++c)
 //     tree[c].draw_foci();

    for (ent::set::const_iterator i = all.begin(); i != all.end(); ++i)
        (*i)->draw_foci();
}

//-----------------------------------------------------------------------------

