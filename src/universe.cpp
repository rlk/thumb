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
#include "conf.hpp"
#include "host.hpp"
#include "user.hpp"

//-----------------------------------------------------------------------------

uni::universe::universe()
{
    double r0 = 6372797.0;
    double r1 = 6372797.0 + 8844.0;

    // Configure the data sources.

    std::string c_name = ::conf->get_s("data_dir");
    std::string n_name = ::conf->get_s("data_dir");
    std::string h_name = ::conf->get_s("data_dir");

    int c_lod = ::conf->get_i("image_color_lod");
    int n_lod = ::conf->get_i("image_normal_lod");
    int h_lod = ::conf->get_i("image_height_lod");

    c_name.append("earth/earth-color/earth-color");
    n_name.append("earth/earth-normal/earth-normal");
    h_name.append("earth/earth-height/earth-height");

    if (c_lod == 0) c_lod = DEFAULT_COLOR_LOD;
    if (n_lod == 0) n_lod = DEFAULT_NORMAL_LOD;
    if (h_lod == 0) h_lod = DEFAULT_HEIGHT_LOD;

    color  = new geomap(c_name.c_str(), 86400, 43200, 3, 1, 512, pow(2, c_lod),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);
    normal = new geomap(n_name.c_str(), 86400, 43200, 3, 1, 512, pow(2, n_lod),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);
    height = new geomap(h_name.c_str(), 86400, 43200, 1, 2, 512, pow(2, h_lod),
                        r0, r1, -PI, PI, -PI / 2, PI / 2);

    // Configure the geometry generator and renderer.

    double patch_bias  = ::conf->get_i("patch_bias");
    int    patch_cache = ::conf->get_i("patch_cache");
    int    patch_depth = ::conf->get_i("patch_depth");

    if (patch_bias == 0)
        patch_bias = DEFAULT_PATCH_BIAS;
    if (patch_cache == 0)
        patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0)
        patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    // Create the earth.

    S[0] = new sphere(*D, *R, *color, *normal, *height,
                      r0, r1, patch_bias, patch_cache);
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

    load_mat(M, ::user->get_M());
    load_mat(I, ::user->get_I());

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

        n = S[i]->min_d();
        f = S[i]->max_d();

        // HACK: far should be outside the geomap bounds
/*
        ::user->range(n / 4.0, f * 2.0);
        ::user->draw();
*/
        glLoadIdentity();

        S[i]->draw(frag_d, frag_k);
    }
}

double uni::universe::rate() const
{
    return S[0] ? S[0]->altitude() : 1.0;
}

void uni::universe::turn(double d)
{
    if (S[0]) S[0]->turn(d * 4.0);
}

//-----------------------------------------------------------------------------
