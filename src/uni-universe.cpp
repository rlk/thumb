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

#include "uni-universe.hpp"
#include "uni-geomap.hpp"
#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-conf.hpp"
#include "app-host.hpp"
#include "app-user.hpp"

//-----------------------------------------------------------------------------

uni::universe::universe()
{
    double Er0 = 6372797.0;
    double Er1 = 6372797.0 + 8844.0;

    double Mr0 = 1737100.0;
    double Mr1 = 1737100.0;

    // Create the caches.

    int image_cache_w = std::max(::conf->get_i("image_cache_w"), 2);
    int image_cache_h = std::max(::conf->get_i("image_cache_h"), 2);

    int height_cache_w = std::max(::conf->get_i("height_cache_w"), 2);
    int height_cache_h = std::max(::conf->get_i("height_cache_h"), 2);

    geocsh *cache_s = new geocsh(3, 1, 510, image_cache_w,
                                            image_cache_h);
    geocsh *cache_h = new geocsh(1, 2, 510, height_cache_w,
                                            height_cache_h);

    caches.push_back(cache_s);
    caches.push_back(cache_h);

    // Create the maps.
/*
    geomap *dif0 = new geomap(cache_s, "world.200408.xml",      Er0, Er1);
    geomap *nrm0 = new geomap(cache_s, "srtm_ramp2_normal.xml", Er0, Er1);
    geomap *nrm1 = new geomap(cache_s, "NED_norm.xml",          Er0, Er1);
    geomap *hgt0 = new geomap(cache_h, "srtm_ramp2.xml",        Er0, Er1);
    geomap *hgt1 = new geomap(cache_h, "NED.xml",               Er0, Er1);

    Ecolor.push_back(dif0);
    Enormal.push_back(nrm0);
    Enormal.push_back(nrm1);
    Eheight.push_back(hgt0);
    Eheight.push_back(hgt1);
*/
    geomap *dif2 = new geomap(cache_s, "moon-750.xml",          Mr0, Mr1);
    geomap *nrm2 = new geomap(cache_s, "moon-normal.xml",       Mr0, Mr1);
    geomap *hgt2 = new geomap(cache_h, "moon-height.xml",       Mr0, Mr1);

    Mcolor.push_back(dif2);
    Mnormal.push_back(nrm2);
    Mheight.push_back(hgt2);

    // Configure the geometry generator and renderer.

    int patch_cache = ::conf->get_i("patch_cache");
    int patch_depth = ::conf->get_i("patch_depth");

    if (patch_cache == 0) patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0) patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    // Create the galaxy.

    G = new galaxy("hipparcos.bin");

    // Create the Earth.
/*
    S[1] = new sphere(*D, *R, Ecolor, Enormal, Eheight,
                      caches, Er0, Er1, patch_cache, true);
    S[1]->move(-384400000.0, 0.0, -Er0 * 2.0);
*/
//  S[1]->move(0.0, 0.0, -Er0 * 2.0);

    // Create the Moon.

    S[0] = new sphere(*D, *R, Mcolor, Mnormal, Mheight,
                      caches, Mr0, Mr1, patch_cache, false);
//  S[0]->move(384400000.0, 0.0, -Er0 * 2.0);
    S[0]->move(0.0, 0.0, -Er0 * 2.0);

    N = 1;
}

uni::universe::~universe()
{
    for (int s = 0; s < N; ++s)
        delete S[s];

    delete G;
    delete R;
    delete D;

    while (!Eheight.empty()) { delete Eheight.front(); Eheight.pop_front(); }
    while (!Enormal.empty()) { delete Enormal.front(); Enormal.pop_front(); }
    while (! Ecolor.empty()) { delete  Ecolor.front();  Ecolor.pop_front(); }

    while (!Mheight.empty()) { delete Mheight.front(); Mheight.pop_front(); }
    while (!Mnormal.empty()) { delete Mnormal.front(); Mnormal.pop_front(); }
    while (! Mcolor.empty()) { delete  Mcolor.front();  Mcolor.pop_front(); }

    while (!caches.empty()) { delete caches.front(); caches.pop_front(); }
}

//-----------------------------------------------------------------------------

void uni::universe::prep(app::frustum_v& frusta)
{
    int s;

    // Preprocess all objects.

    G->view(frusta);

    for (s = 0; s < N; ++s) S[s]->view(frusta);

    std::sort(S + 0, S + N, sphcmp);

    for (s = 0; s < N; ++s) S[s]->step();
    for (s = 0; s < N; ++s) S[s]->prep();
}

void uni::universe::draw(int i)
{
    int s;

    // Draw all objects.

    G->draw(i);

    for (s = 0; s < N; ++s) S[s]->draw(i);
}

double uni::universe::turn_rate() const
{
    double a = S[0] ? S[0]->altitude() : 1.0;

    double k = a * 360.0 / 1e8;

    return std::min(k, 30.0);
}

double uni::universe::move_rate() const
{
    return S[0] ? S[0]->altitude() : 1.0;
}

void uni::universe::turn(double a, double t)
{
    if (S[0]) S[0]->turn(a * 5.0, t * 5.0);
}

//-----------------------------------------------------------------------------
