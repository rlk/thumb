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

#include "default.hpp"
#include "app-tile.hpp"
#include "app-normal.hpp"
#include "app-dome.hpp"
#include "app-varrier.hpp"

//-----------------------------------------------------------------------------

app::tile::tile(app::node node) : current(0)
{
    app::node curr;

    int window[4];

    window[0] = 0;
    window[1] = 0;
    window[2] = DEFAULT_PIXEL_WIDTH;
    window[3] = DEFAULT_PIXEL_HEIGHT;

    // Check for view and tile indices.

    index = get_attr_d(node, "index", 0);

    // Extract the window viewport rectangle.

    if ((curr = find(node, "viewport")))
    {
        window[0] = get_attr_d(curr, "x");
        window[1] = get_attr_d(curr, "y");
        window[2] = get_attr_d(curr, "w", DEFAULT_PIXEL_WIDTH);
        window[3] = get_attr_d(curr, "h", DEFAULT_PIXEL_HEIGHT);
    }

    // Extract all display configurations.

    for (curr = find(node,       "disp"); curr;
         curr = next(node, curr, "disp"))
    {
        const char *t = get_attr_s(curr, "type", "normal");

        if (strcmp(t, "normal")   == 0)
            display.push_back(new app::normal  (node, curr, window));
        if (strcmp(t, "dome")     == 0)
            display.push_back(new app::dome    (node, curr, window));
        if (strcmp(t, "varrier")  == 0)
            display.push_back(new app::varrier (node, curr, window));
/*
        if (strcmp(t, "anaglyph") == 0)
            display.push_back(new app::anaglyph(node, curr, window));
*/
    }

    // Create a fall-back normal display.

    display.push_back(new app::normal(node, 0, window));
}

app::tile::~tile()
{
    // TODO: release the display objects.
}

//-----------------------------------------------------------------------------

bool app::tile::input_point(int i, const double *p, const double *q)
{
    if (display[current] &&
        display[current]->input_point(i, p, q))
        return true;
    else
        return false;
}

bool app::tile::input_click(int i, int b, int m, bool d)
{
    if (display[current] &&
        display[current]->input_click(i, b, m, d))
        return true;
    else
        return false;
}

bool app::tile::input_keybd(int c, int k, int m, bool d)
{
    if (display[current] &&
        display[current]->input_keybd(c, k, m, d))
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------

void app::tile::prep(view_v& views, frustum_v& frusta)
{
    if (display[current])
        display[current]->prep(views, frusta);
}

void app::tile::draw(view_v& views, int& c, bool calibrate_state,
                                            int  calibrate_index)
{
    bool calibrate = calibrate_state && ((calibrate_index == index) ||
                                         (calibrate_index == -1));

    if (display[current])
        display[current]->draw(views, c, calibrate);
}

//-----------------------------------------------------------------------------

bool app::tile::pick(double *p, double *q, int x, int y)
{
    if (display[current])
        return display[current]->pick(p, q, x, y);
    else
        return false;
}

//-----------------------------------------------------------------------------

