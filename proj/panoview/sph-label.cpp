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

sph_label::sph_label(const void *data_ptr, size_t data_len,
                     const void *font_ptr, size_t font_len) :
    ring(make_ring(64)),
    mark(make_mark())
{
    items.push_back(new point(mark, 1, 1));
    items.push_back(new point(ring, 2, 2));
    items.push_back(new circle(ring, 0, 0, 500 / 1737.4));
}

sph_label::~sph_label()
{
    std::vector<label *>::iterator i;

    for (i = items.begin(); i != items.end(); ++i)
        delete (*i);

    glDeleteLists(mark, 1);
    glDeleteLists(ring, 1);
}

void sph_label::draw(const double *p, double r, double a)
{
    glUseProgram(0);

    glPushAttrib(GL_ENABLE_BIT);
    {
        std::vector<label *>::iterator i;

        double k;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (i = items.begin(); i != items.end(); ++i)
        {
            if ((k = (*i)->view(p, r, a)) > 0.0)
            {
                glColor4f(0.0f, 0.0f, 0.0f, k);
                (*i)->draw(p, r, a);
            }
        }
    }
    glPopAttrib();
}

//------------------------------------------------------------------------------

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
