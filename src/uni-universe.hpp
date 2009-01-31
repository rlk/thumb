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
#include "uni-slides.hpp"
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
        sphere *M;
        galaxy *G;
        slides *Z;

        int N;
        int serial;

        double time;

    public:

        universe(int, int);
       ~universe();

        void prep(app::frustum_v&);
        void draw(int);

        void script(const char *, char *);

        double turn_rate() const;
        double move_rate() const;
        double head_dist() const;

        double get_time() const   { return time; }
        void   set_time(double t) { time = t;    }
    };
}

//-----------------------------------------------------------------------------

#endif
