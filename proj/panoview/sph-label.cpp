//  Copyright (C) 2005-2012 Robert Kooima
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
#include <cstdlib>
#include <cstring>
#include <regex.h>

#include <GL/glew.h>

#include "sph-label.hpp"
#include "math3d.h"
#include "type.h"
#include "glsl.h"

//------------------------------------------------------------------------------

static GLuint make_ring(int n)
{
    GLuint o = glGenLists(1);

    glNewList(o, GL_COMPILE);
    {
        glBegin(GL_LINE_LOOP);
        {
            for (int i = 0; i < n; ++i)
                glVertex2d(0.5 * cos(2.0 * M_PI * i / n),
                           0.5 * sin(2.0 * M_PI * i / n));
        }
        glEnd();
    }
    glEndList();

    return o;
}

static GLuint make_mark()
{
    GLuint o = glGenLists(1);

    glNewList(o, GL_COMPILE);
    {
        glBegin(GL_LINES);
        {
            glVertex2d(-0.05, 0.0);
            glVertex2d(+0.05, 0.0);
            glVertex2d(0.0, -0.05);
            glVertex2d(0.0, +0.05);
        }
        glEnd();
    }
    glEndList();

    return o;
}

//------------------------------------------------------------------------------

struct matrix
{
    double M[16];

    matrix()
    {
        midentity(M);
    }
    void rotatey(double a)
    {
        double A[16], B[16];
        mrotatey(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
    void rotatex(double a)
    {
        double A[16], B[16];
        mrotatex(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
    void rotatez(double a)
    {
        double A[16], B[16];
        mrotatez(A, a);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
    void translate(double x, double y, double z)
    {
        double A[16], B[16], v[3] = { x, y, z };
        mtranslate(A, v);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
    void scale(double k)
    {
        double A[16], B[16], v[3] = { k, k, k };
        mscale(A, v);
        mmultiply(B, M, A);
        mcpy(M, B);
    }
};

#if 0
static void set_matrix(double *M, double lon, double lat, double rad, int len)
{
    double t[3] = { 0.f, 0.f, 1.f };
    double s[3] = { 0.001f, 0.001f, 0.001f };
    double L[16];
    double P[16];
    double T[16];
    double S[16];

    mrotatey(L,  radians(lon));
    mrotatex(P, -radians(lat));
    mtranslate(T, t);
    mscale    (S, s);

    mmultiply(M, L, P);
    mmultiply(L, M, T);
    mmultiply(M, L, S);
}
#endif

//------------------------------------------------------------------------------

sph_label::sph_label(const void *data_ptr, size_t data_len,
                     const void *font_ptr, size_t font_len) :
    ring(make_ring(64)),
    mark(make_mark())
{
    const double r = 1737400.0;
    std::vector<char *> strv;
    std::vector<matrix> matv;

    parse(data_ptr, data_len);

    label_font = font_create(font_ptr, font_len, 64, 1.0);

    for (int i = 0; i < int(labels.size()); ++i)
    {
        int len = line_length(labels[i].str, label_font);

        double k = labels[i].dia / r;
        matrix M;

        k = std::min(k, 0.0005);
        k = std::max(k, 0.00001);

        M.rotatey  ( radians(labels[i].lon));
        M.rotatex  (-radians(labels[i].lat));
        M.translate(0.0, 0.0, 1.0);
        M.scale    (k);
        M.translate(-len / 2.0, 0.0, 0.0);

        strv.push_back(labels[i].str);
        matv.push_back(M);
    }

    label_line = line_layout(strv.size(), &strv.front(), NULL,
                                           matv.front().M, label_font);
}

sph_label::~sph_label()
{
    line_delete(label_line);
    font_delete(label_font);

    // std::vector<label *>::iterator i;

    // for (i = items.begin(); i != items.end(); ++i)
    //     delete (*i);

    glDeleteLists(mark, 1);
    glDeleteLists(ring, 1);
}

//------------------------------------------------------------------------------

#define PATTERN "\"([^\"]*)\",([^,]*),([^,]*),([^,]*)\n"

void sph_label::parse(const void *data_ptr, size_t data_len)
{
    regmatch_t match[5];
    regex_t    regex;
    int err;

    const char *dat = (const char *) data_ptr;

    if ((err = regcomp(&regex, PATTERN, REG_EXTENDED)) == 0)
    {
        while ((err = regexec(&regex, dat, 5, match, 0)) == 0)
        {
            label L;

            memset (L.str, 0, strmax);
            strncpy(L.str, dat + match[1].rm_so, match[1].rm_eo-match[1].rm_so);
            L.lat = strtod(dat + match[2].rm_so, NULL);
            L.lon = strtod(dat + match[3].rm_so, NULL);
            L.dia = strtod(dat + match[4].rm_so, NULL);

            labels.push_back(L);

            dat = (const char *) dat + match[0].rm_eo;
        }
    }
}

void sph_label::draw(const double *p, double r, double a)
{
    glUseProgram(0);

    glPushAttrib(GL_ENABLE_BIT);
    {
        // std::vector<label *>::iterator i;

        // double k;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // for (i = items.begin(); i != items.end(); ++i)
        // {
        //     if ((k = (*i)->view(p, r, a)) > 0.0)
        //     {
        //         glColor4f(0.0f, 0.0f, 0.0f, k);
        //         (*i)->draw(p, r, a);
        //     }
        // }

        glEnable(GL_TEXTURE_2D);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        line_render(label_line);
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------
#if 0
void sph_label::point::draw(const double *v, double r, double a)
{
    double k = 1.0 / 1000.0;
    double s = 0.2 * r;

    glPushMatrix();
    {
        glRotated(l,  0, 1, 0);
        glRotated(p, -1, 0, 0);
        glTranslated(0, 0, r);
        glScaled(s, s, s);

        glDisable(GL_TEXTURE_2D);
        glCallList(o);
        glEnable(GL_TEXTURE_2D);

#if 0
        if (str)
        {
            glScaled(k, k, k);
            glTranslated(16.0, 0.0, 0.0);

            str->draw();
        }
#endif
    }
    glPopMatrix();
}

void sph_label::circle::draw(const double *v, double r, double a)
{
    double k = 1.0 / 1000.0;
    double s = d * r;
    double z = r - r * sqrt(4.0 - d * d) / 2.0;

    glPushMatrix();
    {
        glRotated(l,  0, 1, 0);
        glRotated(p, -1, 0, 0);
        glTranslated(0, 0, r);

        glPushMatrix();
        {
            glTranslated(0.0, 0.0, -z);
            glScaled(s, s, s);
            glDisable(GL_TEXTURE_2D);
            glCallList(o);
            glEnable(GL_TEXTURE_2D);
        }
        glPopMatrix();
#if 0
        if (str)
        {
            glScaled(s, s, s);
            glScaled(k, k, k);
            glTranslated(-str->w() / 2.0,
                         -str->h() / 2.0, 0.0);
            str->draw();
        }
#endif
    }
    glPopMatrix();
}

double sph_label::label::view(const double *v, double r, double a)
{
    return 1.0;

    double u[3];

    u[0] = sin(radians(l)) * cos(radians(p));
    u[1] =                   sin(radians(p));
    u[2] = cos(radians(l)) * cos(radians(p));

    double k = vdot(u, v) * 2.0 - 1.0;

    if (k > 0.0)
        return 3.0 * k * k - 2.0 * k * k * k;
    else
        return 0.0;
}
#endif