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

#include "uni-geomap.hpp"
#include "app-serial.hpp"
#include "matrix.hpp"
#include "app-glob.hpp"

/*
static void dump(const GLubyte *v)
{
    int w = 32;
    int h = 16;

    for (int i = 0; i < h; ++i)
    {
        for (int j = 0; j < w; ++j)
        {
            int r = 256 -  1 - i;
            int c = 512 - 32 + j;

            printf("%02x ", v[r * 512 + c]);
        }
        printf("\n");
    }
    printf("\n");
}
*/
//-----------------------------------------------------------------------------

uni::page::page(int w, int h, int s,
                int x, int y, int d,
                double _W, double _E, double _S, double _N) :
    d(d), a(0), live(true)
{
    int t = s << d;

    // Compute the mipmap file indices.

    i = y / t;
    j = x / t;

    // Compute the texture boundries.

    double W, E, S, N;

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
        sphere_to_vector(v, cW, 0, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, 0, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }

    {
        sphere_to_vector(v, cW, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cW, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, cS, 1.0); a = std::max(a, acos(DOT3(v, n)));
        sphere_to_vector(v, cE, cN, 1.0); a = std::max(a, acos(DOT3(v, n)));
    }

    f = ((cE - cW) * (cN - cS)) / ((E - W) * (N - S));

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

    if (live)
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

void uni::page::draw(double r0, double r1)
{
/*
    double rr = (r0 + r1) * 0.5;

    static const GLfloat color[8][3] = {
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
*/
}

double uni::page::angle(const double *v, double r)
{
    double aa = 0.0;

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

        aa = 2.0 * M_PI * (1.0 - cos(atan(R / D)));
    }

    return aa / f;
}

//-----------------------------------------------------------------------------

uni::geomap::geomap(std::string name, double r0, double r1) :
    d(0), r0(r0), r1(r1), P(0), dirty(false),
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

    image = new GLubyte[512 * 256];
    memset(image, 0xFF, 512 * 256);
}

uni::geomap::~geomap()
{
    if (image) delete [] image;

    if (P) delete P;
}

//-----------------------------------------------------------------------------

GLubyte& uni::geomap::index_x(int d, int i, int j)
{
    int dj = 256 >> d;
    int di = 128 >> d;

    int jj = j + 512 - (dj << 1);
    int ii = i + 256 - (di << 1);

    return image[(ii + di) * 512 + (jj     )];
}

GLubyte& uni::geomap::index_y(int d, int i, int j)
{
    int dj = 256 >> d;
    int di = 128 >> d;

    int jj = j + 512 - (dj << 1);
    int ii = i + 256 - (di << 1);

    return image[(ii     ) * 512 + (jj     )];
}

GLubyte& uni::geomap::index_l(int d, int i, int j)
{
    int dj = 256 >> d;
    int di = 128 >> d;

    int jj = j + 512 - (dj << 1);
    int ii = i + 256 - (di << 1);

    return image[(ii     ) * 512 + (jj + dj)];
}

//-----------------------------------------------------------------------------

void uni::geomap::do_index(int d, int i, int j,
                             GLubyte x, GLubyte y, GLubyte l)
{
    if (d >= 0)
    {
        int j2 = j << 1;
        int i2 = i << 1;

        if (index_l(d, i, j) > l)
        {
            index_l(d, i, j) = l;
            index_x(d, i, j) = x;
            index_y(d, i, j) = y;
        }

        do_index(d - 1, i2 + 0, j2 + 0, x, y, l);
        do_index(d - 1, i2 + 1, j2 + 0, x, y, l);
        do_index(d - 1, i2 + 0, j2 + 1, x, y, l);
        do_index(d - 1, i2 + 1, j2 + 1, x, y, l);
    }
}

void uni::geomap::do_eject(int d, int i, int j,
                           GLubyte x0, GLubyte y0, GLubyte l0,
                           GLubyte x1, GLubyte y1, GLubyte l1)
{
    if (d >= 0)
    {
        int j2 = j << 1;
        int i2 = i << 1;

        if (index_x(d, i, j) == x0 &&
            index_y(d, i, j) == y0 &&
            index_l(d, i, j) == l0)
        {
            index_x(d, i, j) = x1;
            index_y(d, i, j) = y1;
            index_l(d, i, j) = l1;
        }

        do_eject(d - 1, i2 + 0, j2 + 0, x0, y0, l0, x1, y1, l1);
        do_eject(d - 1, i2 + 1, j2 + 0, x0, y0, l0, x1, y1, l1);
        do_eject(d - 1, i2 + 0, j2 + 1, x0, y0, l0, x1, y1, l1);
        do_eject(d - 1, i2 + 1, j2 + 1, x0, y0, l0, x1, y1, l1);
    }
}

void uni::geomap::cache_page(const page *Q, int x, int y)
{
    int d = Q->get_d();
    int i = Q->get_i();
    int j = Q->get_j();

    if (d < 8)
    {
        do_index(d, i, j, GLubyte(x), GLubyte(y), GLubyte(d));
//      dump(image);
        dirty = true;
    }
}

void uni::geomap::eject_page(const page *Q, int x, int y)
{
    int d = Q->get_d();
    int i = Q->get_i();
    int j = Q->get_j();

    if (d < 8)
    {
        GLubyte x1 = (d < 7) ? index_x(d + 1, i >> 1, j >> 1) : 0xFF;
        GLubyte y1 = (d < 7) ? index_y(d + 1, i >> 1, j >> 1) : 0xFF;
        GLubyte l1 = (d < 7) ? index_l(d + 1, i >> 1, j >> 1) : 0xFF;

        do_eject(d, i, j, GLubyte(x),
                          GLubyte(y),
                          GLubyte(d), x1, y1, l1);

//      dump(image);
        dirty = true;
    }
}

void uni::geomap::proc()
{
    if (dirty)
        index->blit(image, 0, 0, 512, 256);

    dirty = false;
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
