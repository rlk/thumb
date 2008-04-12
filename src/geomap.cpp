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

//static int count;

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

    if (cS < 0 && 0 < cN)
    {
        sphere_to_vector(v, W, cV, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, E, cV, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }
    else
    {
        sphere_to_vector(v, W, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, W, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, E, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, E, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }

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
    // TODO: cache this result.

    for (app::frustum_i i = frusta.begin(); i != frusta.end(); ++i)
        if ((*i)->test_cap(n, a, r0, r1) >= 0)
        {
            if (d == 0) return true;

            if (P[0] && P[0]->view(frusta, r0, r1)) return true;
            if (P[1] && P[1]->view(frusta, r0, r1)) return true;
            if (P[2] && P[2]->view(frusta, r0, r1)) return true;
            if (P[3] && P[3]->view(frusta, r0, r1)) return true;

            return false;
        }
    
    return false;
}

void uni::page::draw(app::frustum_v& frusta, double r0, double r1)
{
    double rr = r0 + 10000.0;

    GLfloat color[8][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.5f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
    };

    glColor3fv(color[d]);

    // Draw the cap bound.

    double M[16];

    load_idt(M);

    M[8]  = n[0];
    M[9]  = n[1];
    M[10] = n[2];

    crossprod(M + 4, M + 8, M + 0);
    normalize(M + 4);
    crossprod(M + 0, M + 4, M + 8);
    normalize(M + 0);

    for (int c = 0; c < 128; ++c)
    {
        glPushMatrix();
        {
            glMultMatrixd(M);
            glRotated(360.0 * c / 128.0, 0.0, 0.0, 1.0);
            glRotated(DEG(a),            1.0, 0.0, 0.0);

            glBegin(GL_POINTS);
            {
                glVertex3d(0.0, 0.0, rr);
            }
            glEnd();
        }
        glPopMatrix();
    }

    // Draw the border.

    glBegin(GL_LINE_LOOP);
    {
        const double cW = std::max(W, -M_PI);
        const double cE = std::min(E,  M_PI);
        const double cS = std::max(S, -M_PI_2);
        const double cN = std::min(N,  M_PI_2);

        double v[3], k = 0.01, o = 0.001;

        for (double x = cW + o; x < cE - o; x += k)
        {
            sphere_to_vector(v, x, cS + o, rr);
            glVertex3dv(v);
        }
        for (double x = cS + o; x < cN - o; x += k)
        {
            sphere_to_vector(v, cE - o, x, rr);
            glVertex3dv(v);
        }
        for (double x = cE - o; x > cW + o; x -= k)
        {
            sphere_to_vector(v, x, cN - o, rr);
            glVertex3dv(v);
        }
        for (double x = cN - o; x > cS + o; x -= k)
        {
            sphere_to_vector(v, cW + o, x, rr);
            glVertex3dv(v);
        }
    }
    glEnd();
}

double uni::page::angle(const double *v, double r)
{
    // Leaves return zero because they can't be subdivided anyway.

    if (d > 0)
    {
        double D, R;

        // If a page covers most of the sphere, just use the sphere itself.

        if (a > M_PI_2)
        {
            D = sqrt(DOT3(v, v));
            R = r;
        }
        else
        {
            double p[3];

            p[0] = n[0] * r - v[0];
            p[1] = n[1] * r - v[1];
            p[2] = n[2] * r - v[2];

            D = sqrt(DOT3(p, p));
            R = tan(a) * r;
        }

        // Compute the solid angle of the given radius and distance.

        return 2.0 * M_PI * (1.0 - cos(atan(R / D)));
    }
    return 0.0;
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name, double r0, double r1) :
    d(0), r0(r0), r1(r1), P(0),
    index(glob->new_image(512, 256, GL_TEXTURE_2D, GL_LUMINANCE8, GL_LUMINANCE))
{
    app::serial file(name.c_str());
    
    if (app::node map = app::find(file.get_head(), "map"))
    {
        // Load the map configuration from the file.

        pattern = app::get_attr_s(map, "name");

        double W = app::get_attr_f(map, "W", -M_PI);
        double E = app::get_attr_f(map, "E",  M_PI);
        double S = app::get_attr_f(map, "S", -M_PI_2);
        double N = app::get_attr_f(map, "N",  M_PI_2);

        w = app::get_attr_d(map, "w", 1024);
        h = app::get_attr_d(map, "h", 512);
        s = app::get_attr_d(map, "s", 510);
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

void uni::geomap::index_page(int d, int i, int j, const GLubyte *p)
{
    int dj = 256 >> d;
    int di = 128 >> d;

    j += 512 - dj * 2;
    i += 256 - di * 2;

    index->blit(p + 0, j,      i + di, 1, 1);
    index->blit(p + 1, j,      i,      1, 1);
    index->blit(p + 2, j + dj, i,      1, 1);

/*
    int di = 128;
    int dj = 256;

    if (d < 8)
    {
        while (d > 0)
        {
            i += di;
            j += dj;

            di /= 2;
            dj /= 2;

            d--;
        }

        index->blit(p + 0, j,      i + di, 1, 1);
        index->blit(p + 1, j,      i,      1, 1);
        index->blit(p + 2, j + di, i,      1, 1);
    }
*/
}

void uni::geomap::cache_page(const page *Q, int x, int y)
{
    int d = Q->get_d();
    int i = Q->get_i();
    int j = Q->get_j();

    GLubyte p[3];

    p[0] = GLubyte(x);
    p[1] = GLubyte(y);
    p[2] = 0xFF;

    index_page(d, i, j, p);
}

void uni::geomap::eject_page(const page *Q, int x, int y)
{
    int d = Q->get_d();
    int i = Q->get_i();
    int j = Q->get_j();

    GLubyte p[3];

    p[0] = 0x00;
    p[1] = 0x00;
    p[2] = 0x00;

    // This only needs to update the D pixel.

    index_page(d, i, j, p);
}

//-----------------------------------------------------------------------------

void uni::geomap::draw()
{
    index->bind(GL_TEXTURE1);
    {
        ogl::program::current->uniform("index", 1);
        ogl::program::current->uniform("cache", 2);

        ogl::program::current->uniform("data_size", w, h);
        ogl::program::current->uniform("page_size", s + 2, s + 2);
        ogl::program::current->uniform("tile_size", s, s);

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
    index->free(GL_TEXTURE1);
}

std::string uni::geomap::name(const page *P)
{
    char str[256];

    // Construct the file name.

    sprintf(str, pattern.c_str(), P->get_d(), P->get_i(), P->get_j());

    return std::string(str);
}

//-----------------------------------------------------------------------------
