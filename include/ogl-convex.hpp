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

// The convex object represensts a 3D convex hull in forms compatible with
// the ODE convex collider geometry and ODE trimesh mass calculator.
//
// The number of polygons always equals the number of planes.

namespace ogl
{
    class convex
    {
        std::string               name;
        std::vector<double>       points;
        std::vector<double>       planes;
        std::vector<unsigned int> polygons;
        std::vector<unsigned int> indices;

    public:

        convex(std::string name);

        const std::string& get_name() const { return name; }

        double       *get_points  () { return   &points.front(); }
        double       *get_planes  () { return   &planes.front(); }
        unsigned int *get_polygons() { return &polygons.front(); }
        unsigned int *get_indices () { return  &indices.front(); }

        int   num_points  () const   { return    points.size() / 3; }
        int   num_planes  () const   { return    planes.size() / 4; }
        int   num_polygons() const   { return    planes.size() / 4; }
        int   num_indices () const   { return   indices.size();     }

        int   siz_points  () const   { return 3 * sizeof (double);       }
        int   siz_indices () const   { return 3 * sizeof (unsigned int); }
    };
}

//-----------------------------------------------------------------------------

#endif
