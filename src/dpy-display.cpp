//  Copyright (C) 2008 Robert Kooima
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
#include "ogl-program.hpp"
#include "dpy-display.hpp"

//-----------------------------------------------------------------------------

dpy::display::display(app::node node)
{
    app::node curr;

    // Check for view and tile indices.

    index = app::get_attr_d(node, "index", 0);

    // Extract the window viewport rectangle.

    if ((curr = app::find(node, "viewport")))
    {
        viewport[0] = app::get_attr_d(curr, "x");
        viewport[1] = app::get_attr_d(curr, "y");
        viewport[2] = app::get_attr_d(curr, "w", DEFAULT_PIXEL_WIDTH);
        viewport[3] = app::get_attr_d(curr, "h", DEFAULT_PIXEL_HEIGHT);
    }
    else
    {
        viewport[0] = 0;
        viewport[1] = 0;
        viewport[2] = DEFAULT_PIXEL_WIDTH;
        viewport[3] = DEFAULT_PIXEL_HEIGHT;
    }
}

//-----------------------------------------------------------------------------

void dpy::display::fill(double screen_w, double screen_h,
                        int    buffer_w, int    buffer_h) const
{
    // Draw a screen-filling quad using screen-space vertices.

    const double sw = 0.5 * screen_w;
    const double sh = 0.5 * screen_h;

    const double kx = double(buffer_w) / double(viewport[2]);
    const double ky = double(buffer_h) / double(viewport[3]);

    glViewport(viewport[0],
               viewport[1],
               viewport[2],
               viewport[3]);

    // Apply the screen-space projection.

    glMatrixMode(GL_PROJECTION);
    {
        glPushMatrix();
        glLoadIdentity();
        glOrtho(-sw, +sw, -sh, +sh, -1.0, +1.0);
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPushMatrix();
        glLoadIdentity();
    }

    // Load uniforms to map from fragment coordinates to buffer coordinates.

    if (const ogl::program *P = ogl::program::current)
    {
        P->uniform("frag_d", -viewport[0] * kx, -viewport[1] * ky);
        P->uniform("frag_k",                kx,                ky);
    }

    // Draw the screen-space quad.

    glBegin(GL_QUADS);
    {
        glVertex2d(-sw, -sh);
        glVertex2d(+sw, -sh);
        glVertex2d(+sw, +sh);
        glVertex2d(-sw, +sh);
    }
    glEnd();

    // Revert the projection state.

    glMatrixMode(GL_PROJECTION);
    {
        glPopMatrix();
    }
    glMatrixMode(GL_MODELVIEW);
    {
        glPopMatrix();
    }
}

//-----------------------------------------------------------------------------
