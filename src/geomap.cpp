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

static int count;

//-----------------------------------------------------------------------------

uni::page::page(int w, int h, int s,
                int x, int y, int d,
                double _W, double _E, double _S, double _N) : d(d), a(0)
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

    // Compute the cap normal.

    const double cW = std::max(W, -M_PI);
    const double cE = std::min(E,  M_PI);
    const double cS = std::max(S, -M_PI_2);
    const double cN = std::min(N,  M_PI_2);

    const double cH = (cW + cE) / 2;
    const double cV = (cS + cN) / 2;

    sphere_to_vector(n, cH, cV, 1.0);

    // Compute the cap angle.

    double v[3];

    if (S < M_PI_2 && M_PI_2 < N)
    {
        sphere_to_vector(v, cW, cV, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, cV, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }
    else
    {
        sphere_to_vector(v, cW, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cW, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }

    if (x == 0 && y == 0)
        printf("%d %f %f %f %f %f %f %f %f\n", d,
               DEG(cW), DEG(cE), DEG(cS), DEG(cN),
               n[0], n[1], n[2], DEG(a));

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

bool uni::page::view(app::frustum_v& frusta, double r0, double r1)
{
    for (app::frustum_i i = frusta.begin(); i != frusta.end(); ++i)
        if ((*i)->test_cap(n, a, r0, r1) >= 0)
            return true;
    
    return false;
}

void uni::page::draw(app::frustum_v& frusta, double r0, double r1)
{
    if (d == 4 && view(frusta, r0, r1))
    {
        count++;

        double M[16];

        load_idt(M);

        M[8]  = n[0];
        M[9]  = n[1];
        M[10] = n[2];

        crossprod(M + 4, M + 8, M + 0);
        normalize(M + 4);
        crossprod(M + 0, M + 4, M + 8);
        normalize(M + 0);

        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

        for (int c = 0; c < 128; ++c)
        {
            glPushMatrix();
            {
                glMultMatrixd(M);
                glRotated(360.0 * c / 128.0, 0.0, 0.0, 1.0);
                glRotated(DEG(a),            1.0, 0.0, 0.0);

                glBegin(GL_POINTS);
                {
                    glVertex3d(0.0, 0.0, r0);
                }
                glEnd();
            }
            glPopMatrix();
        }

        glBegin(GL_LINE_LOOP);
        {
            const double cW = std::max(W, -M_PI);
            const double cE = std::min(E,  M_PI);
            const double cS = std::max(S, -M_PI_2);
            const double cN = std::min(N,  M_PI_2);

            double v[3], k = 0.01;

            for (double x = cW; x < cE; x += k)
            {
                sphere_to_vector(v, x, cS, r0);
                glVertex3dv(v);
            }
            for (double x = cS; x < cN; x += k)
            {
                sphere_to_vector(v, cE, x, r0);
                glVertex3dv(v);
            }
            for (double x = cE; x > cW; x -= k)
            {
                sphere_to_vector(v, x, cN, r0);
                glVertex3dv(v);
            }
            for (double x = cN; x > cS; x -= k)
            {
                sphere_to_vector(v, cW, x, r0);
                glVertex3dv(v);
            }
        }
        glEnd();
    }

    {
        if (P[0]) P[0]->draw(frusta, r0, r1);
/*
        if (P[1]) P[1]->draw(frusta, r0, r1);
        if (P[2]) P[2]->draw(frusta, r0, r1);
        if (P[3]) P[3]->draw(frusta, r0, r1);
*/
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

void uni::geomap::draw(app::frustum_v& frusta, double r0, double r1)
{
    count = 0;

    if (P) P->draw(frusta, r0, r1);

    printf("%d\n", count);

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
