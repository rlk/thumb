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

#ifndef OCT_HPP
#define OCT_HPP

#include "entity.hpp"

//-----------------------------------------------------------------------------

namespace ent
{
    class oct
    {
    private:

        //---------------------------------------------------------------------
        // Octree cell

        struct cell
        {
            ent::set entities;

            void draw_fill();
            void draw_foci();
        };

        //---------------------------------------------------------------------
        // Octree

        int up(int c)        const { return (c - 1) >> 3;      }
        int dn(int p, int i) const { return (8 * p) + (i + 1); }

        ent::set all;

        float range;
        int   depth;
        int   count;
        cell *tree;

        void aabb(float[6], int) const;
        bool test(entity *, int) const;
        void move(entity *, int);
        bool push(entity *);
        bool pull(entity *);
        void seek(entity *);

    public:

        typedef ent::set::iterator iterator;

        oct(float range=64, int depth=5);
       ~oct();

        void insert(entity *e);
        void remove(entity *e);
        void modify(entity *e);

        iterator begin() { return all.begin(); }
        iterator end()   { return all.end();   }

        void get(ent::set& sel) const { sel = all; }

        void draw_foci();
        void draw_fill();
    };
}

//-----------------------------------------------------------------------------

#endif
