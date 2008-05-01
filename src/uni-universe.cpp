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

    // Configure the data sources.

    cache_s = new geocsh(3, 1, 510, 8, 8);
    cache_h = new geocsh(1, 2, 510, 8, 8);

    color  = new geomap("world.200408.xml", r0, r1);
    normal = new geomap("srtm_ramp2_normal.xml", r0, r1);
    height = new geomap("srtm_ramp2.xml", r0, r1);

    // Configure the geometry generator and renderer.

    int patch_cache = ::conf->get_i("patch_cache");
    int patch_depth = ::conf->get_i("patch_depth");

    if (patch_cache == 0) patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0) patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    // Create the earth.

    S[0] = new sphere(*D, *R, *cache_s, *cache_h, *color, *normal, *height,
                      r0, r1, patch_cache);
    S[0]->move(0.0, 0.0, -r0 * 2.0);
}

uni::universe::~universe()
{
    if (height) delete height;
    if (normal) delete normal;
    if (color)  delete color;

    if (cache_h)  delete cache_h;
    if (cache_s)  delete cache_s;

    delete S[0];
    delete R;
    delete D;

//  uni::geomap::fini();
}

//-----------------------------------------------------------------------------

void uni::universe::prep(app::frustum_v& frusta)
{
    // Preprocess all objects.

    S[0]->view(frusta);
    S[0]->step();
    S[0]->prep();
}

void uni::universe::draw(int i)
{
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
