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

#include "region.hpp"
#include "opengl.hpp"
#include "util.hpp"

#define MXML_FORALL(t, i, n) \
    for (i = mxmlFindElement((t), (t), (n), 0, 0, MXML_DESCEND); i; \
         i = mxmlFindElement((i), (t), (n), 0, 0, MXML_NO_DESCEND))

//-----------------------------------------------------------------------------

app::region::region(mxml_node_t *node, int w, int h)
    : node(node), w(w), h(h),
      curr_corner(0),
      curr_button(0)
{
    if (node == 0)
    {
        // The default region is a screen-filling quad.

        corners.push_back(corner(0, 0, 0, 0, 0));
        corners.push_back(corner(0, w, 0, w, 0));
        corners.push_back(corner(0, w, h, w, h));
        corners.push_back(corner(0, 0, h, 0, h));
    }
    else
    {
        // Parse the list of corners from the given XML node.

        mxml_node_t *curr;

        MXML_FORALL(node, curr, "corner")
        {
            const char *c;

            int ix = 0;
            int iy = 0;
            int ox = 0;
            int oy = 0;

            if ((c = mxmlElementGetAttr(curr, "ix"))) ix = atoi(c);
            if ((c = mxmlElementGetAttr(curr, "iy"))) iy = atoi(c);
            if ((c = mxmlElementGetAttr(curr, "ox"))) ox = atoi(c);
            if ((c = mxmlElementGetAttr(curr, "oy"))) oy = atoi(c);

            corners.push_back(corner(curr, ix, iy, ox, oy));
        }
    }
}

app::region::~region()
{
}

//-----------------------------------------------------------------------------

void app::region::point(int x, int y)
{
    int X =     x;
    int Y = h - y;

    if (curr_button == 1)
    {
        corners[curr_corner].ix = X;
        corners[curr_corner].iy = Y;

        if (corners[curr_corner].node)
        {
            set_attr_i(corners[curr_corner].node, "ix", X);
            set_attr_i(corners[curr_corner].node, "iy", Y);
        }
    }
    if (curr_button == 3)
    {
        corners[curr_corner].ox = X;
        corners[curr_corner].oy = Y;

        if (corners[curr_corner].node)
        {
            set_attr_i(corners[curr_corner].node, "ox", X);
            set_attr_i(corners[curr_corner].node, "oy", Y);
        }
    }
}

void app::region::click(int b, bool d)
{
    if (d)
        curr_button = b;
    else
        curr_button = 0;
}

void app::region::keybd(int k, int m)
{
    int i = k - '0';

    if (0 <= i && i < int(corners.size()))
        curr_corner = i;
}

//-----------------------------------------------------------------------------

void app::region::draw() const
{
    std::vector<corner>::const_iterator i;

    // Draw the center area of the region.

    glColor3ub(0xFF, 0xFF, 0xFF);

    glBegin(GL_POLYGON);
    {
        for (i = corners.begin(); i != corners.end(); ++i)
            glVertex2i(i->ix, i->iy);
    }
    glEnd();

    // Draw the blended edges of the region.

    glBegin(GL_QUAD_STRIP);
    {
        for (i = corners.begin(); i != corners.end(); ++i)
        {
            glColor3ub(0xFF, 0xFF, 0xFF); glVertex2i(i->ix, i->iy);
            glColor3ub(0x00, 0x00, 0x00); glVertex2i(i->ox, i->oy);
        }
        
        i = corners.begin();

        glColor3ub(0xFF, 0xFF, 0xFF); glVertex2i(i->ix, i->iy);
        glColor3ub(0x00, 0x00, 0x00); glVertex2i(i->ox, i->oy);
    }
    glEnd();
}

void app::region::wire() const
{
    std::vector<corner>::const_iterator i;

    // Draw a wireframe of the region edges.

    glColor3ub(0xFF, 0xFF, 0x00);

    glBegin(GL_LINE_LOOP);
    {
        for (i = corners.begin(); i != corners.end(); ++i)
            glVertex2i(i->ix, i->iy);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    {
        for (i = corners.begin(); i != corners.end(); ++i)
            glVertex2i(i->ox, i->oy);
    }
    glEnd();

    glBegin(GL_LINES);
    {
        for (i = corners.begin(); i != corners.end(); ++i)
        {
            glVertex2i(i->ix, i->iy);
            glVertex2i(i->ox, i->oy);
        }
    }
    glEnd();

    // Draw the active points.

    glPushAttrib(GL_POINT_BIT);
    {
        glPointSize(8.0f);

        glBegin(GL_POINTS);
        {
            glColor3ub(0x00, 0xFF, 0x00);
            glVertex2i(corners[curr_corner].ix, corners[curr_corner].iy);
            glColor3ub(0xFF, 0x00, 0xFF);
            glVertex2i(corners[curr_corner].ox, corners[curr_corner].oy);
        }
        glEnd();
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------
