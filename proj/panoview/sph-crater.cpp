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

#include "sph-crater.hpp"

//------------------------------------------------------------------------------

sph_crater::sph_crater(const std::string& dat)
{
    app::file file(dat);
    app::font sans(::conf->get_s("sans_font"), 64);

    if (app::node p = file.get_root().find("planet"))
    {
        double r = p.get_f("radius");

        for (app::node c = p.find("crater"); c; c = p.next(c, "crater"))
        {
            const std::string& name = c.get_s("name");
            double             d    = c.get_f("diameter");
            double             p    = c.get_f("lat");
            double             l    = c.get_f("lon");

            craters.push_back(new crater(sans, name, d / r, p, l));

            if (d < 100) break;
        }
    }

    glNewList((ring = glGenLists(1)), GL_COMPILE);
    {
        glBegin(GL_LINE_LOOP);
        {
            int n = 64;

            for (int i = 0; i < n; ++i)
                glVertex2d(0.5 * cos(2.0 * M_PI * i / n),
                           0.5 * sin(2.0 * M_PI * i / n));
        }
        glEnd();
    }
    glEndList();
}

sph_crater::~sph_crater()
{
    std::vector<crater *>::iterator i;

    for (i = craters.begin(); i != craters.end(); ++i)
        delete (*i);

    glDeleteLists(ring, 1);
}

void sph_crater::draw(const double *p, double r, double a)
{
    glUseProgram(0);

    glPushAttrib(GL_ENABLE_BIT);
    {
        std::vector<crater *>::iterator i;

        double a;

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (i = craters.begin(); i != craters.end(); ++i)
            if ((a = (*i)->visible(p)) > 0.0)
            {
                app::text *str = (*i)->str;
                double d = (*i)->d * r;
                double l = (*i)->l;
                double p = (*i)->p;
                double k = 1.0 / 1000.0;

                glColor4f(0.0f, 0.0f, 0.0f, a);

                glPushMatrix();
                {
                    glRotated(l,  0, 1, 0);
                    glRotated(p, -1, 0, 0);
                    glTranslated(0, 0, r);
                    glScaled(d, d, d);

                    glDisable(GL_TEXTURE_2D);
                    glCallList(ring);

                    glScaled(k, k, k);
                    glTranslated(-str->w() / 2.0,
                                 -str->h() / 2.0, 0.0);

                    glEnable(GL_TEXTURE_2D);
                    str->draw();
                }
                glPopMatrix();
            }
    }
    glPopAttrib();

}

//------------------------------------------------------------------------------

double sph_crater::crater::visible(const double *u)
{
    double v[3];

    v[0] = sin(RAD(l)) * cos(RAD(p));
    v[1] =               sin(RAD(p));
    v[2] = cos(RAD(l)) * cos(RAD(p));

    double k = DOT3(u, v) * 2.0 - 1.0;

    if (k > 0.0)
        return 3.0 * k * k - 2.0 * k * k * k;
    else
        return 0.0;
}