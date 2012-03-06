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

#include <app-conf.hpp>
#include <app-file.hpp>
#include <ogl-opengl.hpp>

#include "sph-crater.hpp"

//------------------------------------------------------------------------------

sph_crater::sph_crater(const std::string& dat)
{
    app::file file(dat);
    app::font sans(::conf->get_s("sans_font"),
                   ::conf->get_i("sans_size"));

    if (app::node p = file.get_root().find("planet"))
    {
        double r = p.get_f("radius");

        for (app::node c = p.find("crater"); c; c = p.next(c, "crater"))
        {
            const std::string& name = c.get_s("name");
            double             d    = c.get_f("diameter") / r;
            double             p    = c.get_f("lat");
            double             l    = c.get_f("lon");

            craters.push_back(new crater(sans, name, d, p, l));
        }
    }
}

sph_crater::~sph_crater()
{
    std::vector<crater *>::iterator i;

    for (i = craters.begin(); i != craters.end(); ++i)
        delete (*i);
}

void sph_crater::draw(const double *p, double r, double a)
{
    glUseProgram(0);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor3f(0.0f, 1.0f, 0.0f);

        for (int i = 0; i < 100; ++i)
        {
            app::text *str = craters[i]->str;
            double d = craters[i]->d / 50.0;
            double l = craters[i]->l;
            double p = craters[i]->p;

            glPushMatrix();
            {
                glRotated(l, 0, 1, 0);
                glRotated(p, 1, 0, 0);
                glTranslated(0, 0, r);
                glScaled(d, d, d);
                glTranslated(-str->w() / 2.0,
                             -str->h() / 2.0, 0.0);

                str->draw();
            }
            glPopMatrix();
        }
    }
    glPopAttrib();

}

//------------------------------------------------------------------------------
