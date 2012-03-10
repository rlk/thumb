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

#include <etc-math.hpp>
#include <app-conf.hpp>
#include <app-file.hpp>
#include <app-user.hpp>
#include <ogl-opengl.hpp>

#include "sph-label.hpp"

//------------------------------------------------------------------------------

static GLuint make_ring(int n)
{
    GLuint o;

    glNewList((o = glGenLists(1)), GL_COMPILE);
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
    GLuint o;

    glNewList((o = glGenLists(1)), GL_COMPILE);
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

sph_label::sph_label(const std::string& dat) :
    ring(make_ring(64)),
    mark(make_mark())
{
    app::file file(dat);
    app::font sans(::conf->get_s("sans_font"), 64);

    if (app::node p = file.get_root().find("sphere"))
    {
        double r = p.get_f("radius");

        for (app::node c = p.find("point"); c; c = p.next(c, "point"))
        {
            const std::string& name = c.get_s("label");
            double             p    = c.get_f("lat");
            double             l    = c.get_f("lon");
            items.push_back(new point(sans, name, mark, p, l));
        }

        for (app::node c = p.find("circle"); c; c = p.next(c, "circle"))
        {
            const std::string& name = c.get_s("label");
            double             p    = c.get_f("lat");
            double             l    = c.get_f("lon");
            double             d    = c.get_f("diameter");
            items.push_back(new circle(sans, name, ring, p, l, d / r));
        }
    }
}

sph_label::~sph_label()
{
    std::vector<label *>::iterator i;

    for (i = items.begin(); i != items.end(); ++i)
        delete (*i);

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

        if (str)
        {
            glScaled(k, k, k);
            glTranslated(16.0, 0.0, 0.0);

            str->draw();
        }
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

        if (str)
        {
            glScaled(s, s, s);
            glScaled(k, k, k);
            glTranslated(-str->w() / 2.0,
                         -str->h() / 2.0, 0.0);
            str->draw();
        }
    }
    glPopMatrix();
}

double sph_label::label::view(const double *v, double r, double a)
{
    double u[3];

    u[0] = sin(RAD(l)) * cos(RAD(p));
    u[1] =               sin(RAD(p));
    u[2] = cos(RAD(l)) * cos(RAD(p));

    double k = DOT3(u, v) * 2.0 - 1.0;

    if (k > 0.0)
        return 3.0 * k * k - 2.0 * k * k * k;
    else
        return 0.0;
}
