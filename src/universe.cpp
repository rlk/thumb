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

#include <math.h>

#include "universe.hpp"
#include "geomap.hpp"
#include "matrix.hpp"
#include "glob.hpp"
#include "host.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------

#define DATA "/home/evl/rlk/data/"

uni::universe::universe()
{
    double r0 = 6372797.0;
    double r1 = 6372797.0 + 8844.0;

    color  = new geomap(DATA "earth/earth-color/earth-color",
                        86400, 43200, 3, 1, 512, pow(2, 16),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);
    normal = new geomap(DATA "earth/earth-normal/earth-normal",
                        86400, 43200, 3, 1, 512, pow(2, 16),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);
    height = new geomap(DATA "earth/earth-height/earth-height",
                        86400, 43200, 1, 2, 512, pow(2, 16),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);

//  color->set_debug(true);

    D = new geodat();
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    S[0] = new sphere(*D, *R, *color, *normal, *height, r0, r1);
    S[0]->move(0.0, 0.0, -r0 * 2.0);
}

uni::universe::~universe()
{
/*
    delete S[2];
    delete S[1];
*/
    if (height) delete height;
    if (normal) delete normal;
    if (color)  delete color;

    delete S[0];
    delete R;
    delete D;

    uni::geomap::fini();
}

//-----------------------------------------------------------------------------

void uni::universe::prep(const double *F, int n)
{
    // Determine the modelview matrix and inverse.

    double M[16];
    double I[16];

    ::view->get_M(M);

    load_inv(I, M);

    // Preprocess all objects.

    int i, N = 1;

    for (i = 0; i < N; ++i) S[i]->view(M, I, F, n);

    std::sort(S, S + N, sphcmp);
    
    for (i = 0; i < N; ++i) S[i]->step();
    for (i = 0; i < N; ++i) S[i]->prep();
}

void uni::universe::draw(const double *frag_d,
                         const double *frag_k)
{
    int i, N = 1;

    // Draw all objects.

    for (i = 0; i < N; ++i)
    {
        double n;
        double f;

        S[i]->getz(n, f);

        // HACK: far should be outside the geomap bounds

        ::view->range(n / 2.0, f * 2.0);
        ::view->draw();

        glLoadIdentity();

        S[i]->draw(frag_d, frag_k);
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
