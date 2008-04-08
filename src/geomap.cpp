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

#include <cmath>

#include "geomap.hpp"
#include "serial.hpp"
#include "matrix.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

uni::page::page(int w, int h, int s,
                int x, int y, int d,
                double _W, double _E, double _S, double _N) : d(d)
{
    int t = s << d;

    // Compute the mipmap file indices.

    i = y / t;
    j = x / t;

    // Compute the texture boundries.

    W = (_W + (_E - _W) * (x    ) / w);
    E = (_W + (_E - _W) * (x + t) / w);
    S = (_S + (_N - _S) * (y    ) / h);
    N = (_S + (_N - _S) * (y + t) / h);

    // Create subpages as necessary.

    P[0] = 0;
    P[1] = 0;
    P[2] = 0;
    P[3] = 0;

    if (d > 0)
    {
        const int X = (x + t / 2);
        const int Y = (y + t / 2);

        /* Has children? */ P[0] = new page(w, h, s, x, y, d-1, _W, _E, _S, _N);
        if (X < w         ) P[1] = new page(w, h, s, X, y, d-1, _W, _E, _S, _N);
        if (         Y < h) P[2] = new page(w, h, s, x, Y, d-1, _W, _E, _S, _N);
        if (X < w && Y < h) P[3] = new page(w, h, s, X, Y, d-1, _W, _E, _S, _N);
    }
}

uni::page::~page()
{
    if (P[3]) delete P[3];
    if (P[2]) delete P[2];
    if (P[1]) delete P[1];
    if (P[0]) delete P[0];
}

void uni::page::draw(double r)
{
    if (d == 0)
    {
        const GLfloat c0[4] = { 1.0f, 1.0f, 0.0f, 0.5f };
        const GLfloat c1[4] = { 1.0f, 1.0f, 0.0f, 0.0f };

        double r0 = r;
        double r1 = r + 10000.0;

        double nw[3];
        double sw[3];
        double ne[3];
        double se[3];

        sphere_to_vector(nw, W, N, 1);
        sphere_to_vector(sw, W, S, 1);
        sphere_to_vector(ne, E, N, 1);
        sphere_to_vector(se, E, S, 1);

        glBegin(GL_QUAD_STRIP);
        {
            glColor4fv(c0); glVertex3d(nw[0] * r0, nw[1] * r0, nw[2] * r0);
            glColor4fv(c1); glVertex3d(nw[0] * r1, nw[1] * r1, nw[2] * r1);

            glColor4fv(c0); glVertex3d(sw[0] * r0, sw[1] * r0, sw[2] * r0);
            glColor4fv(c1); glVertex3d(sw[0] * r1, sw[1] * r1, sw[2] * r1);

            glColor4fv(c0); glVertex3d(se[0] * r0, se[1] * r0, se[2] * r0);
            glColor4fv(c1); glVertex3d(se[0] * r1, se[1] * r1, se[2] * r1);

            glColor4fv(c0); glVertex3d(ne[0] * r0, ne[1] * r0, ne[2] * r0);
            glColor4fv(c1); glVertex3d(ne[0] * r1, ne[1] * r1, ne[2] * r1);
        }
        glEnd();
    }
    else
    {
        if (P[0]) P[0]->draw(r);
        if (P[1]) P[1]->draw(r);
        if (P[2]) P[2]->draw(r);
        if (P[3]) P[3]->draw(r);
    }
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name, double r0, double r1) :
    name(name), r0(r0), r1(r1), P(0)
{
    app::serial file(name.c_str());
    
    if (app::node map = app::find(file.get_head(), "map"))
    {
        // Load the map configuration from the file.

        double W = app::get_attr_f(map, "W", -M_PI);
        double E = app::get_attr_f(map, "E",  M_PI);
        double S = app::get_attr_f(map, "S", -M_PI_2);
        double N = app::get_attr_f(map, "N",  M_PI_2);

        int w = app::get_attr_d(map, "w", 1024);
        int h = app::get_attr_d(map, "h", 512);

        s = app::get_attr_d(map, "s", 512);
        c = app::get_attr_d(map, "c", 3);
        b = app::get_attr_d(map, "b", 1);

        // Compute the depth of the mipmap pyramid.

        int t = s;

        while (t < w || t < h)
        {
            t *= 2;
            d += 1;
        }

        // Generate the mipmap pyramid catalog.

        P = new page(w, h, s, 0, 0, d, W, E, S, N);
    }
}

uni::geomap::~geomap()
{
    if (P) delete P;
}

//-----------------------------------------------------------------------------

void uni::geomap::draw(double r)
{
    if (P) P->draw(r);
/*
    index->bind(GL_TEXTURE1);
    cache->bind(GL_TEXTURE2);
    {
        ogl::program::current->uniform("index", 1);
        ogl::program::current->uniform("cache", 2);

        ogl::program::current->uniform("data_size", 86400.0, 43200.0);
        ogl::program::current->uniform("page_size",   512.0,   512.0);

        glMatrixMode(GL_PROJECTION);
        {
            glPushMatrix();
            glLoadIdentity();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPushMatrix();
            glLoadIdentity();
        }

        glRecti(-1, -1, +1, +1);

        glMatrixMode(GL_PROJECTION);
        {
            glPopMatrix();
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glPopMatrix();
        }
    }
    cache->free(GL_TEXTURE2);
    index->free(GL_TEXTURE1);
*/
}

//-----------------------------------------------------------------------------
