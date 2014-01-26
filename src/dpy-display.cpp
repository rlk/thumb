//  Copyright (C) 2008-2011 Robert Kooima
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

#include <app-default.hpp>
#include <ogl-program.hpp>
#include <dpy-display.hpp>

//-----------------------------------------------------------------------------

dpy::display::display(app::node p)
{
    // Check for view and tile indices.

    index = p.get_i("index", 0);

    // Extract the window viewport rectangle.

    if (app::node n = p.find("viewport"))
    {
        viewport[0] = n.get_i("x", 0);
        viewport[1] = n.get_i("y", 0);
        viewport[2] = n.get_i("w", DEFAULT_PIXEL_WIDTH);
        viewport[3] = n.get_i("h", DEFAULT_PIXEL_HEIGHT);
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

    const double bw = double(buffer_w);
    const double bh = double(buffer_h);

    const double kx = bw / double(viewport[2]);
    const double ky = bh / double(viewport[3]);

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
    glActiveTexture(GL_TEXTURE0);

    // Load uniforms to map from fragment coordinates to buffer coordinates.

    if (const ogl::program *P = ogl::program::current)
    {
        P->uniform("frag_d", vec2(-viewport[0] * kx, -viewport[1] * ky));
        P->uniform("frag_k", vec2(               kx,                ky));
    }

    // Draw the screen-space quad.

    glBegin(GL_QUADS);
    {
        glTexCoord2d(-1, -1); glVertex2d(-sw, -sh);
        glTexCoord2d(+1, -1); glVertex2d(+sw, -sh);
        glTexCoord2d(+1, +1); glVertex2d(+sw, +sh);
        glTexCoord2d(-1, +1); glVertex2d(-sw, +sh);
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
