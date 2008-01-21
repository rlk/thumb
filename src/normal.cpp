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

#include "normal.hpp"
#include "user.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

app::normal::normal(app::node tile, app::node node) : frust(0), prog(0)
{
    app::node curr;

    // Check the display definition for a frustum.

    if      ((curr = find(node, "frustum")))
        frust = new app::frustum(curr);

    // If none, check the tile definition for one.

    else if ((curr = find(tile, "frustum")))
        frust = new app::frustum(curr);

    // Note the view index.

    index = get_attr_d(node, "view");
}

app::normal::~normal()
{
    if (frust) delete frust;
}

//-----------------------------------------------------------------------------

void app::normal::prep(view_v& views, frustum_v& frusta)
{
    if (frust)
    {
        // Apply the viewpoint and view to my frustum.

        frust->set_view(views[index]->get_p(), ::user->get_I());

        // Add my frustum to the list.

        frusta.push_back(frust);
    }

    // Ensure the draw shader is initialized.

    if (prog == 0 && (prog = ::glob->load_program("glsl/normal.vert",
                                                  "glsl/normal.frag")))
    {
        prog->bind();
        {
            prog->uniform("map", 0);
            prog->uniform("frag_d", 0.0, 0.0);
            prog->uniform("frag_k", 1.0, 1.0);
        }
        prog->free();
    }
}

void app::normal::draw(view_v& views, bool calibrate)
{
    if (views[index])
    {
        // Draw the scene to the off-screen buffer.

        views[index]->bind();
        {
            const GLubyte *c = views[index]->get_c();

            glPushAttrib(GL_COLOR_BUFFER_BIT);
            {
                glClearColor(c[0], c[1], c[2], c[3]);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            glPopAttrib();

//          ::prog->draw(frust);
        }
        views[index]->free();

        // Draw the off-screen buffer to the screen.

        if (prog)
        {
            views[index]->bind_color(GL_TEXTURE0);
            prog->bind();
            {
                glBegin(GL_QUADS);
                {
                    glVertex2f(-1.0f, -1.0f);
                    glVertex2f(+1.0f, -1.0f);
                    glVertex2f(+1.0f, +1.0f);
                    glVertex2f(-1.0f, +1.0f);
                }
                glEnd();
            }
            prog->free();
            views[index]->free_color(GL_TEXTURE0);
        }
    }
}

//-----------------------------------------------------------------------------
