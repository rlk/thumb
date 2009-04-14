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

#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-user.hpp"
#include "uni-slides.hpp"

//=============================================================================

uni::slide::slide(app::node n) :
    texture(::glob->load_texture(n.get_s("image"),
                                 n.get_s("image")))
{
    // Extract the slide transformation from the node.

    double p[3];
    double q[4];

    p[0] = n.get_f("x", 0);
    p[1] = n.get_f("y", 0);
    p[2] = n.get_f("z", 0);
    q[0] = n.get_f("t", 0);
    q[1] = n.get_f("u", 0);
    q[2] = n.get_f("v", 0);
    q[3] = n.get_f("w", 0);

    set_quaternion(M, q);

    M[12] = p[0];
    M[13] = p[1];
    M[14] = p[2];
}

uni::slide::~slide()
{
    if (texture) ::glob->free_texture(texture);
}

//-----------------------------------------------------------------------------

void uni::slide::draw(const double *p) const
{
    if (texture && distance(p, M + 12) < 100.0)
    {
        glPushAttrib(GL_ENABLE_BIT);
        glPushMatrix();
        {
            glMultMatrixd(M);

            glRotatef(30.0f, 1.0f, 0.0f, 0.0f);

//          HACK gett getw geth have been removed.
//          glEnable(texture->gett());

            texture->bind();
            {
                glBegin(GL_QUADS);
                {
                    int w = 1920; // texture->getw();
                    int h = 1080; // texture->geth();

                    double a = double(w) / double(h);

                    glColor3f(1.0f, 1.0f, 1.0f);

                    glTexCoord2f(0, 0); glVertex3f(-a, -1, -2);
                    glTexCoord2f(w, 0); glVertex3f(+a, -1, -2);
                    glTexCoord2f(w, h); glVertex3f(+a, +1, -2);
                    glTexCoord2f(0, h); glVertex3f(-a, +1, -2);
                }
                glEnd();
            }
            texture->free();
        }
        glPopMatrix();
        glPopAttrib();
    }
}

//=============================================================================

uni::slides::slides(std::string filename) :
    file(filename.c_str())
{
    app::node p;

    if (app::node p = file.get_root().find("slides"))
    {
        // Create a slide object for each slide node.

        for (app::node n = p.find("slide"); n; n = p.next(n, "slide"))
            all.push_back(new slide(n));
    }
}

uni::slides::~slides()
{
    // Delete all slides.

    for (slide_i i = all.begin(); i != all.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

void uni::slides::view(app::frustum_v& frusta)
{
    // Store the current user transform for use during drawing.

    load_mat(M, ::user->get_I());
    load_mat(I, ::user->get_M());

    while (!this->frusta.empty())
    {
        delete this->frusta.back();
        this->frusta.pop_back();
    }

    // Apply the transform to the frusta.

    for (int i = 0; i < int(frusta.size()); ++i)
    {
        app::frustum *frust = new app::frustum(*(frusta[i]));

        frust->set_transform(I);

        this->frusta.push_back(frust);
    }
}

void uni::slides::draw(int i) const
{
    // Apply the projection.

    frusta[i]->set_distances(1.0, 1000.0);
    frusta[i]->draw();

    glPushAttrib(GL_ENABLE_BIT);
    glPushMatrix();
    {
        const double *p = frusta[i]->get_view_pos();

        // Set up the GL state for slide rendering.

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        // Apply the current user transform.

        glLoadMatrixd(M);

        // Draw all slides.

        for (slide_c i = all.begin(); i != all.end(); ++i)
            (*i)->draw(p);
    }
    glPopMatrix();
    glPopAttrib();
}

//=============================================================================
