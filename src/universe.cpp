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

    cache  = new geocsh(3, 1, 510, 8, 4);
    color  = new geomap("world.200408.xml", r0, r1);
    normal = 0;
    height = 0;

    // Configure the geometry generator and renderer.

    int patch_cache = ::conf->get_i("patch_cache");
    int patch_depth = ::conf->get_i("patch_depth");

    if (patch_cache == 0) patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0) patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(::host->get_buffer_w(),
                   ::host->get_buffer_h());

    // Create the earth.

    S[0] = new sphere(*D, *R, *cache, *color, *normal, *height,
                      r0, r1, patch_cache);
    S[0]->move(0.0, 0.0, -r0 * 2.0);
}

uni::universe::~universe()
{
    if (height) delete height;
    if (normal) delete normal;
    if (color)  delete color;
    if (cache)  delete cache;

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

double uni::universe::rate() const
{
    return S[0] ? S[0]->altitude() : 1.0;
}

void uni::universe::turn(double a, double t)
{
    if (S[0]) S[0]->turn(a * 5.0, t * 5.0);
}

//-----------------------------------------------------------------------------
