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

#include "universe.hpp"
#include "glob.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

uni::universe::universe()
{
    color = glob->load_texture("texture/earth-color.png");
    terra = glob->load_texture("texture/earth-terra.png");

    terra->bind();
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    }
    terra->free();

    color->bind();
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
    }
    color->free();

    D = new geodat();
    R = new georen(::view->get_w(),
                   ::view->get_h());

/*
    S[0] = new sphere(*D, color, terra,   6372797.0,   6372797.0 + 8844.0);
    S[1] = new sphere(*D, color, terra,   1737103.0,   1737103.0);
    S[2] = new sphere(*D, color, terra, 696000000.0, 696000000.0);

    S[0]->move(        0.0, 0.0, -149597887500.0);
    S[1]->move(384400000.0, 0.0, -149597887500.0);
*/
    S[0] = new sphere(*D, *R, color, terra,   6372797.0,   6372797.0 + 8844.0);
    S[0]->move(0.0, 0.0, -6372797.0 * 2.0);

}

uni::universe::~universe()
{
    glob->free_texture(terra);
    glob->free_texture(color);
/*
    delete S[2];
    delete S[1];
*/
    delete S[0];
    delete R;
    delete D;
}

//-----------------------------------------------------------------------------

void uni::universe::draw()
{
    int i, n = 1;

    // Preprocess all objects.

    for (i = 0; i < n; ++i) S[i]->view();

    std::sort(S, S + n, sphcmp);
    
    for (i = 0; i < n; ++i) S[i]->step();
    for (i = 0; i < n; ++i) S[i]->prep();

    // Apply the light source.

    glPushMatrix();
    {
        float L[4] = { 0.0f, 0.0f, 1.0f, 0.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, L);
    }
    glPopMatrix();

    // Draw all objects.

    for (i = 0; i < n; ++i)
    {
        glClear(GL_DEPTH_BUFFER_BIT);

        glPushMatrix();
        {
            double n;
            double f;

            S[i]->getz(n, f);

            ::view->range(n / 2.0, f);

            glMatrixMode(GL_PROJECTION);
            {
                glLoadIdentity();
                ::view->draw();
            }
            glMatrixMode(GL_MODELVIEW);
            {
                glLoadIdentity();
            }

            S[i]->draw();
        }
        glPopMatrix();
    }
}

double uni::universe::rate() const
{
    return S[0] ? S[0]->altitude() : 1.0;
}

//-----------------------------------------------------------------------------
