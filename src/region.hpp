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

#ifndef REGION_HPP
#define REGION_HPP

#include <vector>
#include <mxml.h>

//-----------------------------------------------------------------------------

namespace app
{
    class region
    {
        // XML region element

        mxml_node_t *node;
        int w;
        int h;

        // XML corner elements

        struct corner
        {
            mxml_node_t *node;
            int ix;
            int iy;
            int ox;
            int oy;

            corner(mxml_node_t *node, int ix, int iy, int ox, int oy) :
                node(node), ix(ix), iy(iy), ox(ox), oy(oy) { }
        };

        std::vector<corner> corners;

        // Editing state

        int curr_point;

    public:

        region(mxml_node_t *, int, int);
       ~region();

        void point(int, int);
        void click(int, bool);
        void keybd(int, int);

        void draw() const;
        void wire() const;
    };
}

//-----------------------------------------------------------------------------

#endif
