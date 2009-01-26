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
#include "dpy-normal.hpp"
/*
#include "dpy-dome.hpp"
#include "dpy-varrier.hpp"
#include "dpy-anaglyph.hpp"
*/

//-----------------------------------------------------------------------------

app::tile::tile(app::node node) : display(0)
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

    // Extract a display configurations.

    if ((curr = find(node, "display")))
    {
        const char *t = get_attr_s(curr, "type", "normal");

        if (strcmp(t, "normal")   == 0)
            display.push_back(new dpy::normal  (node, curr, window));
/*
        if (strcmp(t, "dome")     == 0)
            display.push_back(new dpy::dome    (node, curr, window));
        if (strcmp(t, "varrier")  == 0)
            display.push_back(new dpy::varrier (node, curr, window));
        if (strcmp(t, "anaglyph") == 0)
            display.push_back(new dpy::anaglyph(node, curr, window));
*/
    }
}

app::tile::~tile()
{
    // TODO: release the display objects.
}

//-----------------------------------------------------------------------------

void app::tile::get_frusta(frustum_v& frusta)
{
    if (display)
        display->get_frusta(frusta);
}

void app::tile::draw(view *views, int frusti)
{
    if (display)
        display->draw(views, frusti)
}

void app::tile::test(view *views, int calibi)
{
    if (display)
        display->test(views, frusti)
}

//-----------------------------------------------------------------------------

bool app::tile::project_event(app::event *E, int x, int y)
{
    if (display[current])
        return display[current]->project_event(E, x, y);
    else
        return false;
}

bool app::tile::process_event(app::event *E)
{
    if (display[current])
        return display[current]->process_event(E);
    else
        return false;
}

//-----------------------------------------------------------------------------

