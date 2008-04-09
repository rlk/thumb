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

#ifndef GEOMAP
#define GEOMAP

#include "texture.hpp"
#include "frustum.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    //-------------------------------------------------------------------------
    // Geomap page

    class page
    {
        int    d;
        int    i;
        int    j;
        double W;
        double E;
        double S;
        double N;

        double n[3];
        double a;

        page *P[4];

    public:

        page(int, int, int, int, int, int, double, double, double, double);
       ~page();

        bool view(app::frustum_v&, double, double);
        void draw(app::frustum_v&, double, double);
    };

    //-------------------------------------------------------------------------
    // Geomap

    class geomap
    {
        std::string name;

        int    c;
        int    b;
        int    s;
        int    d;
        double r0;
        double r1;

        page *P;

    public:

        geomap(std::string, double, double);
       ~geomap();

        void draw(app::frustum_v&, double, double);
    };
}

//-----------------------------------------------------------------------------

#endif
