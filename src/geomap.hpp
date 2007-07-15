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

//-----------------------------------------------------------------------------

namespace uni
{
    class tile
    {
        tile  *C[4];
        double n[3];
        double a;
        double k;
        int    i;
        int    j;
        GLuint o;

    public:

        tile();
       ~tile();

    };

    class geomap
    {
        std::string name;

        int w;
        int h;
        int s;
        int c;
        int b;

        tile *T;
        tile *Q;

    public:

        geomap(std::string, int, int, int, int);
       ~geomap();

    };
}

//-----------------------------------------------------------------------------

#endif
