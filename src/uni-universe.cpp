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
    double r0 = 6372797.0;
    double r1 = 6372797.0 + 8844.0;

    int image_cache_w = std::max(::conf->get_i("image_cache_w"), 2);
    int image_cache_h = std::max(::conf->get_i("image_cache_h"), 2);

    int height_cache_w = std::max(::conf->get_i("height_cache_w"), 2);
    int height_cache_h = std::max(::conf->get_i("height_cache_h"), 2);

    // Create the caches.

    geocsh *cache_s = new geocsh(3, 1, 510, image_cache_w,
                                            image_cache_h);
    geocsh *cache_h = new geocsh(1, 2, 510, height_cache_w,
                                            height_cache_h);

    caches.push_back(cache_s);
    caches.push_back(cache_h);

    // Create the maps.

    geomap *dif0 = new geomap(cache_s, "world.200408.xml",      r0, r1);
    geomap *nrm0 = new geomap(cache_s, "srtm_ramp2_normal.xml", r0, r1);
    geomap *nrm1 = new geomap(cache_s, "NED_norm.xml",          r0, r1);
    geomap *hgt0 = new geomap(cache_h, "srtm_ramp2.xml",        r0, r1);
    geomap *hgt1 = new geomap(cache_h, "NED.xml",               r0, r1);

    color.push_back(dif0);
    normal.push_back(nrm0);
    normal.push_back(nrm1);
    height.push_back(hgt0);
    height.push_back(hgt1);

    // Configure the geometry generator and renderer.

    int patch_cache = ::conf->get_i("patch_cache");
    int patch_depth = ::conf->get_i("patch_depth");

    if (patch_cache == 0) patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0) patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    // Create the earth.

    S[0] = new sphere(*D, *R, color, normal, height, caches,
                      r0, r1, patch_cache);
    S[0]->move(0.0, 0.0, -r0 * 2.0);

    // Create the galaxy.

    G = new galaxy("hipparcos.bin");
}

uni::universe::~universe()
{
    delete G;
    delete S[0];
    delete R;
    delete D;

    while (!height.empty()) { delete height.front(); height.pop_front(); }
    while (!normal.empty()) { delete normal.front(); normal.pop_front(); }
    while (! color.empty()) { delete  color.front();  color.pop_front(); }
    while (!caches.empty()) { delete caches.front(); caches.pop_front(); }
}

//-----------------------------------------------------------------------------

void uni::universe::prep(app::frustum_v& frusta)
{
    // Preprocess all objects.

    G->view(frusta);

    S[0]->view(frusta);
    S[0]->step();
    S[0]->prep();
}

void uni::universe::draw(int i)
{
    G->draw(i);

    S[0]->draw(i);
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
