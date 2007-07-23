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
#include "geomap.hpp"
#include "matrix.hpp"
#include "glob.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

uni::universe::universe()
{
    double r0 = 6372797.0;
    double r1 = 6372797.0 + 8844.0;

    color  = new geomap("/data/earth/earth-color-200408/earth-color-200408",
                        86400, 43200, 3, 1, 1024, r0, r1, -PI, PI, -PI / 2, PI / 2);
    normal = new geomap("/data/earth/earth-normal/earth-normal",
                        86400, 43200, 3, 1, 1024, r0, r1, -PI, PI, -PI / 2, PI / 2);
    height = new geomap("/data/earth/earth-height/earth-normal",
                        86400, 43200, 3, 1, 1024, r0, r1, -PI, PI, -PI / 2, PI / 2);

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
    S[0] = new sphere(*D, *R, *color, *normal, *height, r0, r1);
    S[0]->move(0.0, 0.0, -r0 * 2.0);

}

uni::universe::~universe()
{
/*
    delete S[2];
    delete S[1];
*/
    delete height;
    delete normal;
    delete color;
    delete S[0];
    delete R;
    delete D;
}

//-----------------------------------------------------------------------------

void uni::universe::draw()
{
    double M[16];
    double I[16];
    double P[16];

    int i, n = 1;

    ::view->get_M(M);
    load_inv(I, M);
    ::view->get_P(P);

    // Preprocess all objects.

    for (i = 0; i < n; ++i) S[i]->view(P, M, I);

    std::sort(S, S + n, sphcmp);
    
    for (i = 0; i < n; ++i) S[i]->step();
    for (i = 0; i < n; ++i) S[i]->prep();

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

void uni::universe::turn(int d)
{
    if (S[0]) S[0]->turn(d * 10.0);
}

//-----------------------------------------------------------------------------
