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

#ifndef UNIVERSE_HPP
#define UNIVERSE_HPP

#include "uni-sphere.hpp"
#include "uni-galaxy.hpp"
#include "uni-geomap.hpp"
#include "uni-geocsh.hpp"
#include "app-frustum.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class universe
    {
        geodat *D;
        georen *R;

        geomap_l Ecolor;
        geomap_l Enormal;
        geomap_l Eheight;

        geomap_l Mcolor;
        geomap_l Mnormal;
        geomap_l Mheight;

        geocsh_l caches;

        sphere *S[2];
        galaxy *G;

        int N;
        int serial;

    public:

        universe();
       ~universe();

        void prep(app::frustum_v&);
        void draw(int);

        double turn_rate() const;
        double move_rate() const;
        double head_dist() const;

        void turn(double=0, double=0);

        const double *get_p() const { return S[0] ? S[0]->get_p() : 0; }

        double get_a() const   { return S[0] ? S[0]->get_a( ) : 0; }
        void   set_a(double k) {    if (S[0])  S[0]->set_a(k);     }
        double get_t() const   { return S[0] ? S[0]->get_t( ) : 0; }
        void   set_t(double k) {    if (S[0])  S[0]->set_t(k);     }
    };
}

//-----------------------------------------------------------------------------

#endif
