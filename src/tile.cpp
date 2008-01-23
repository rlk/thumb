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
#include "tile.hpp"
#include "normal.hpp"

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

        if (strcmp(t, "normal")  == 0)
            display.push_back(new app::normal  (node, curr, window));
/*
        if (strcmp(t, "anaglyph") == 0)
            display[i] = new app::anaglyph(node, curr, window);
        if (strcmp(t, "varrier")  == 0)
            display[i] = new app::varrier (node, curr, window);
        if (strcmp(t, "dome")     == 0)
            display[i] = new app::dome    (node, curr, window);
*/
    }
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
                                            int calibrate_index)
{
    bool calibrate = calibrate_state;

    if (display[current])
        display[current]->draw(views, c, calibrate);
}

//-----------------------------------------------------------------------------

bool app::tile::pick(double *p, double *v, int x, int y)
{
/*
    // Determine whether the given pointer position lies within this tile.

    if (window_rect[0] <= x && x < window_rect[0] + window_rect[2] &&
        window_rect[1] <= y && y < window_rect[1] + window_rect[3])
    {
        double kx = double(x - window_rect[0]) / double(window_rect[2]);
        double ky = double(y - window_rect[1]) / double(window_rect[3]);

        // It does.  Compute the eye-space vector given by the pointer.

        p[0] = 0.0;
        p[1] = 0.0;
        p[2] = 0.0;

        v[0] = TL[0] * (1 - kx - ky) + TR[0] * kx + BL[0] * ky;
        v[1] = TL[1] * (1 - kx - ky) + TR[1] * kx + BL[1] * ky;
        v[2] = TL[2] * (1 - kx - ky) + TR[2] * kx + BL[2] * ky;

        // TODO:  Which frustum?  Which view?

        normalize(v);

        return true;
    }
*/
    return false;
}

/*
void app::tile::draw(view_v& views, bool calibrate_state,
                                     int calibrate_index)
{
    const bool focus = (calibrate_index == tile_index);

    // Apply the tile corners.

    double bl[3], br[3], tl[3], tr[3];

    get_BL(bl);
    get_BR(br);
    get_TL(tl);
    get_TR(tr);

    ::user->set_V(bl, br, tl, tr);

    // Render the view from each view.

    std::vector<view *>::iterator i;
    int e;

    for (e = 0, i = views.begin(); i != views.end(); ++i, ++e)
        if (view_index < 0 || view_index == e)
            (*i)->draw(window, focus);

    // Render the onscreen exposure.

    if (const ogl::program *prog = ::user->get_prog())
    {
        double frag_d[2] = { 0, 0 };
        double frag_k[2] = { 1, 1 };

        int w = ::host->get_buffer_w();
        int h = ::host->get_buffer_h();
        int t;

        // Bind the view buffers.  Apply the Varrier transform for each.

        for (t = 0, i = views.begin(); i != views.end(); ++i, ++t)
        {
            (*i)->bind(GL_TEXTURE0 + t);
        
            glActiveTextureARB(GL_TEXTURE0 + t);
            apply_varrier((*i)->get_P());
            glActiveTextureARB(GL_TEXTURE0);
        }

        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(window_rect[0]);
        frag_d[1] =            -double(window_rect[1]);
        frag_k[0] = double(w) / double(window_rect[2]);
        frag_k[1] = double(h) / double(window_rect[3]);

        // Draw the tile region.

        glViewport(window_rect[0], window_rect[1],
                   window_rect[2], window_rect[3]);

        glMatrixMode(GL_PROJECTION);
        {
            // HACK: breaks varrier

            glLoadIdentity();
            glOrtho(0, window_rect[2],
                    0, window_rect[3], -1, +1);

//          glOrtho(-W / 2, +W / 2, -H / 2, +H / 2, -1, +1);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
        }

        prog->bind();
        {
            prog->uniform("L_map", 0);
            prog->uniform("R_map", 1);

            prog->uniform("cycle", varrier_cycle);
            prog->uniform("offset", -W / (3 * window_rect[2]), 0,
                                     W / (3 * window_rect[2]));

            prog->uniform("frag_d", frag_d[0], frag_d[1]);
            prog->uniform("frag_k", frag_k[0], frag_k[1]);

            if (reg) reg->draw();
        }
        prog->free();

        // Free the view buffers...

        for (t = GL_TEXTURE0, i = views.begin(); i != views.end(); ++i, ++t)
            (*i)->free(t);
    }
}
*/

//-----------------------------------------------------------------------------

