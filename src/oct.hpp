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
        //---------------------------------------------------------------------
        // Octree node

        struct node
        {
            ent::set entities;

            void draw_fill() const;
            void draw_foci() const;
        };

        //---------------------------------------------------------------------
        // Octree

    private:

        ent::set all;

        float range;
        int   depth;
        int   count;
        node *tree;

    public:

        typedef ent::set::iterator iterator;

        oct(float range=256, int depth=5);
       ~oct();

        void insert(entity *e);
        void remove(entity *e);

        iterator begin() { return all.begin(); }
        iterator end()   { return all.end();   }

        void get(ent::set& sel) const { sel = all; }

        void draw_foci() const;
        void draw_fill() const;
    };
}

//-----------------------------------------------------------------------------

#endif
