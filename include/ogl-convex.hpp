//  Copyright (C) 2014 Robert Kooima
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

#ifndef OGL_CONVEX_HPP
#define OGL_CONVEX_HPP

#include <string>
#include <vector>

//-----------------------------------------------------------------------------

// The convex object represensts a 3D convex hull in a form compatible with
// the ODE convex collider geometry.

namespace ogl
{
    class convex
    {
        std::string               name;
        std::vector<double>       points;
        std::vector<double>       planes;
        std::vector<unsigned int> polygons;

    public:

        convex(std::string name);

        const std::string& get_name() const { return name; }

        double       *get_points  () { return   &points.front(); }
        double       *get_planes  () { return   &planes.front(); }
        unsigned int *get_polygons() { return &polygons.front(); }

        unsigned int num_points() const { return points.size(); }
        unsigned int num_planes() const { return planes.size(); }
    };
}

//-----------------------------------------------------------------------------

#endif
