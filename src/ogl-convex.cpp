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
    unsigned int j;
    vec3         v;

    // Parse the given string as an OBJ.

    while (getline(data, line))
    {
        std::stringstream cmd(line);
        std::string       key;

        cmd >> key;

        // Read the three coordinates of a vertex.

        if      (key == "v")
        {
            cmd >> v[0] >> v[1] >> v[2];

            points.push_back(double(v[0]));
            points.push_back(double(v[1]));
            points.push_back(double(v[2]));

            vertices.push_back(float(v[0]));
            vertices.push_back(float(v[1]));
            vertices.push_back(float(v[2]));
        }

        // Read the list of indices of a face.

        else if (key == "l")
        {
            std::vector<unsigned int> d;

            while (cmd >> i)
                d.push_back(i - 1);

            // Store the indices as a polygon.

            polygons.insert(polygons.end(), d.size() - 1);
            polygons.insert(polygons.end(), d.begin(), d.end() - 1);

            // Store the indices as an array of triangles.

            for (j = 0; j < d.size() - 3; ++j)
            {
                indices.push_back(d[0]);
                indices.push_back(d[j + 1]);
                indices.push_back(d[j + 2]);
            }
        }
    }

    if (!points.empty())
    {
        // Compute the bounding volume of the point cloud.

        vec3 p;
        vec3 q;

        p[0] = q[0] = points[0];
        p[1] = q[1] = points[1];
        p[2] = q[2] = points[2];

        for (i = 0; i < points.size(); i += 3)
        {
            if (points[i + 0] < p[0]) p[0] = points[i + 0];
            if (points[i + 1] < p[1]) p[1] = points[i + 1];
            if (points[i + 2] < p[2]) p[2] = points[i + 2];

            if (points[i + 0] > q[0]) q[0] = points[i + 0];
            if (points[i + 1] > q[1]) q[1] = points[i + 1];
            if (points[i + 2] > q[2]) q[2] = points[i + 2];
        }

        // Center the point cloud on the origin.

        for (i = 0; i < points.size(); i+= 3)
        {
            points[i + 0] -= (p[0] + q[0]) / 2.0;
            points[i + 1] -= (p[1] + q[1]) / 2.0;
            points[i + 2] -= (p[2] + q[2]) / 2.0;

            vertices[i + 0] -= (p[0] + q[0]) / 2.0;
            vertices[i + 1] -= (p[1] + q[1]) / 2.0;
            vertices[i + 2] -= (p[2] + q[2]) / 2.0;
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
            vec3 w;

            u[0] = points[b * 3 + 0] - points[a * 3 + 0];
            u[1] = points[b * 3 + 1] - points[a * 3 + 1];
            u[2] = points[b * 3 + 2] - points[a * 3 + 2];

            v[0] = points[c * 3 + 0] - points[a * 3 + 0];
            v[1] = points[c * 3 + 1] - points[a * 3 + 1];
            v[2] = points[c * 3 + 2] - points[a * 3 + 2];

            w[0] = u[1] * v[2] - u[2] * v[1];
            w[1] = u[2] * v[0] - u[0] * v[2];
            w[2] = u[0] * v[1] - u[1] * v[0];

            double d = double(sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]));

            w[0] /= d;
            w[1] /= d;
            w[2] /= d;

            planes.push_back(w[0]);
            planes.push_back(w[1]);
            planes.push_back(w[2]);
            planes.push_back(w[0] * points[a * 3 + 0]
                           + w[1] * points[a * 3 + 1]
                           + w[2] * points[a * 3 + 2]);

            i += n + 1;
        }
    }
}

//-----------------------------------------------------------------------------
