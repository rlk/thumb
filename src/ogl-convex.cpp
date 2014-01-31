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

#include <iostream>
#include <sstream>

#include <ogl-convex.hpp>
#include <etc-vector.hpp>
#include <app-data.hpp>

//-----------------------------------------------------------------------------

ogl::convex::convex(std::string name) : name(name)
{
    std::stringstream data((const char *) ::data->load(name));
    std::string       line;

    ::data->free(name);

    unsigned int i;
    vec3         v;

    // Parse the given string as an OBJ.

    while (getline(data, line))
    {
        std::stringstream cmd(line);
        std::string       key;

        cmd >> key;

        if      (key == "v")
        {
            cmd >> v[0] >> v[1] >> v[2];
            points.push_back(v[0]);
            points.push_back(v[1]);
            points.push_back(v[2]);
        }
        else if (key == "l")
        {
            std::vector<unsigned int> indices;

            while (cmd >> i)
                indices.push_back(i - 1);

            polygons.insert(polygons.end(), indices.size() - 1);
            polygons.insert(polygons.end(), indices.begin(), indices.end() - 1);
        }
    }

    if (!points.empty())
    {
        // Compute the bounding volume of the point cloud.

        vec3 a;
        vec3 b;

        a[0] = b[0] = points[0];
        a[1] = b[1] = points[1];
        a[2] = b[2] = points[2];

        for (i = 0; i < points.size(); i += 3)
        {
            if (points[i + 0] < a[0]) a[0] = points[i + 0];
            if (points[i + 1] < a[1]) a[1] = points[i + 1];
            if (points[i + 2] < a[2]) a[2] = points[i + 2];

            if (points[i + 0] > b[0]) b[0] = points[i + 0];
            if (points[i + 1] > b[1]) b[1] = points[i + 1];
            if (points[i + 2] > b[2]) b[2] = points[i + 2];
        }

        // Center the point cloud on the origin.

        for (i = 0; i < points.size(); i+= 3)
        {
            points[i + 0] -= (b[0] + a[0]) / 2;
            points[i + 1] -= (b[1] + a[1]) / 2;
            points[i + 2] -= (b[2] + a[2]) / 2;
        }

        // Compute the plane of each polygon.

        for (i = 0; i < polygons.size();)
        {
            unsigned int n = polygons[i];
            unsigned int a = polygons[i + 1];
            unsigned int b = polygons[i + 2];
            unsigned int c = polygons[i + 3];

            vec3 u;
            vec3 v;
            vec4 p;

            u[0] = points[b * 3 + 0] - points[a * 3 + 0];
            u[1] = points[b * 3 + 1] - points[a * 3 + 1];
            u[2] = points[b * 3 + 2] - points[a * 3 + 2];

            v[0] = points[c * 3 + 0] - points[a * 3 + 0];
            v[1] = points[c * 3 + 1] - points[a * 3 + 1];
            v[2] = points[c * 3 + 2] - points[a * 3 + 2];

            p[0] = u[1] * v[2] - u[2] * v[1];
            p[1] = u[2] * v[0] - u[0] * v[2];
            p[2] = u[0] * v[1] - u[1] * v[0];

            double d = double(sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]));

            p[0] /= d;
            p[1] /= d;
            p[2] /= d;
            p[3]  = p[0] * points[a * 3 + 0]
                  + p[1] * points[a * 3 + 1]
                  + p[2] * points[a * 3 + 2];

            planes.push_back(p[0]);
            planes.push_back(p[1]);
            planes.push_back(p[2]);
            planes.push_back(p[3]);

            i += n + 1;
        }
    }
}

//-----------------------------------------------------------------------------
