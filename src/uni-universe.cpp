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
#include <algorithm>

#include "uni-universe.hpp"
#include "uni-geomap.hpp"
#include "ogl-opengl.hpp"
#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-prog.hpp"

//-----------------------------------------------------------------------------

uni::universe::universe(int w, int h) : G(0), Z(0), serial(0), time(0)
{
    double Er0 =   6372797.0;
    double Er1 =   6372797.0 +  8844.0;
    double Mr0 =   1737100.0;
    double Mr1 =   1737100.0 + 10000.0;
    double Mo  = 384400000.0;

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

    geomap *dif0 = new geomap(cache_s, "universe/world.200408.xml",      Er0, Er1);

    geomap *nrm0 = new geomap(cache_s, "universe/srtm_ramp2_normal.xml", Er0, Er1);
    geomap *hgt0 = new geomap(cache_h, "universe/srtm_ramp2.xml",        Er0, Er1);

    Ecolor.push_back(dif0);
    Enormal.push_back(nrm0);
    Eheight.push_back(hgt0);
/*
    geomap *nrm1 = new geomap(cache_s, "NED_norm.xml",          Er0, Er1);
    geomap *hgt1 = new geomap(cache_h, "NED.xml",               Er0, Er1);

    Enormal.push_back(nrm1);
    Eheight.push_back(hgt1);
*/
    geomap *dif2 = new geomap(cache_s, "universe/moon-750.xml",          Mr0, Mr1);
    geomap *nrm2 = new geomap(cache_s, "universe/moon-normal.xml",       Mr0, Mr1);
    geomap *hgt2 = new geomap(cache_h, "universe/moon-height.xml",       Mr0, Mr1);

    Mcolor.push_back(dif2);
    Mnormal.push_back(nrm2);
    Mheight.push_back(hgt2);

    // Configure the geometry generator and renderer.

    int patch_cache = ::conf->get_i("patch_cache");
    int patch_depth = ::conf->get_i("patch_depth");

    if (patch_cache == 0) patch_cache = DEFAULT_PATCH_CACHE;
    if (patch_depth == 0) patch_depth = DEFAULT_PATCH_DEPTH;

    D = new geodat(patch_depth);
    R = new georen(w, h);

    // Create the galaxy.

    G = new galaxy("hipparcos.bin");
//  Z = new slides("slides.xml");

    // Create the Earth.

    S[0] = new sphere(*D, *R, Ecolor, Enormal, Eheight,
                      caches, Er0, Er1, patch_cache, true, false);
    S[0]->move(-Mo, 0.0, 0.0);
//  S[0]->move(-Mo, 0.0, -2.0 * Er0);
//  S[0]->move(0.0, 0.0, -2.0 * Er0);

    N = 1;

    // Create the Moon.

    S[1] = M = new sphere(*D, *R, Mcolor, Mnormal, Mheight,
                          caches, Mr0, Mr1, patch_cache, false, true);
    S[1]->move(0.0, 0.0, 0.0);
//  S[1]->move(0.0, 0.0, -2.0 * Er0);
//  S[1]->move(+Mo, 0.0, -2.0 * Er0);
//  S[1]->turn(90.0, 0.0);

    N = 2;
}

uni::universe::~universe()
{
    for (int s = 0; s < N; ++s)
        delete S[s];

    if (G) delete G;
    if (Z) delete Z;

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

void uni::universe::prep(int frusc, const app::frustum *const *frusv)
{
    int s;

    serial++;

    ogl::free_texture(); // TODO: eliminate

    // Update the object transforms based on time.

//  if (S[0]) S[0]->set_a(time * 360 / (60.0 * 60.0 * 24.0));

    // Update the view of each object.

    if (G) G->view(frusc, frusv);
//  if (Z) Z->view(frusc, frusv);

    for (s = 0; s < N; ++s)
        S[s]->view(frusc, frusv);

    // Sort the spheres by distance.

    std::sort(S + 0, S + N, sphcmp);

    // Perform visibility processing.

    // HACK: if the GPGPU buffers are shown, step only one sphere.

    if (::prog->get_option(7) ||
        ::prog->get_option(8) ||
        ::prog->get_option(9))
        S[0]->step(serial);
    else
        for (s = 0; s < N; ++s)
            S[s]->step(serial);

    geomap_i m;
    geocsh_i c;
    
    for (c = caches.begin(); c != caches.end(); ++c)
        (*c)->proc(frusc, frusv, serial);

    for (m =  Ecolor.begin(); m !=  Ecolor.end(); ++m) (*m)->proc();
    for (m = Enormal.begin(); m != Enormal.end(); ++m) (*m)->proc();
    for (m = Eheight.begin(); m != Eheight.end(); ++m) (*m)->proc();

    for (m =  Mcolor.begin(); m !=  Mcolor.end(); ++m) (*m)->proc();
    for (m = Mnormal.begin(); m != Mnormal.end(); ++m) (*m)->proc();
    for (m = Mheight.begin(); m != Mheight.end(); ++m) (*m)->proc();

    for (s = 0; s < N; ++s) S[s]->prep();
}

void uni::universe::draw(int i)
{
    int s;

    ogl::free_texture(); // TODO: eliminate

    // Draw all objects.

    if (G) G->draw(i);

    // HACK: if the GPGPU buffers are shown, draw only one sphere.

    if (::prog->get_option(7) ||
        ::prog->get_option(8) ||
        ::prog->get_option(9))
        S[0]->draw(i);
    else
        for (s = N - 1; s >= 0; --s)
            S[s]->draw(i);

    if (Z) Z->draw(i);
}

const char *uni::universe::script(const char *text)
{
    return M->script(text);
}

double uni::universe::get_turn_rate() const
{
    double a = S[0] ? S[0]->altitude() : 1.0;

    double k = a * 360.0 / 1e8;

    return std::min(k, 30.0);
}

double uni::universe::get_move_rate() const
{
    return std::min(1.0e12, S[0] ? S[0]->altitude() : 1.0);
}

//-----------------------------------------------------------------------------
