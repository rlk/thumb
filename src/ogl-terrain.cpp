//  Copyright (C) 2009 Robert Kooima
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

#include "matrix.hpp"
#include "app-data.hpp"
#include "app-file.hpp"
#include "ogl-terrain.hpp"

//-----------------------------------------------------------------------------

struct ogl::head_s
{
    uint32_t c;
    uint16_t n;
    uint16_t m;
};

struct ogl::page_s
{
    int16_t min;
    int16_t max;
    int16_t err;
    int16_t pad;

    uint32_t sub[4];
};

struct ogl::vert_s
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t u;
};

//-----------------------------------------------------------------------------

ogl::terrain::terrain(std::string name) :
    name(name), buff(0), head(0), page(0), vert(0), s(100.0), h(10.0)
{
    app::file file("terrain/" + name);

    // Parse the configuration.

    if (app::node p = file.get_root().find("terrain"))
    {
        data = p.get_s("file");
        s    = p.get_f("size",  100.0);
        h    = p.get_f("height", 10.0);

        // Load the data.

        if (!data.empty())
        {
            if ((buff = (const uint8_t *) ::data->load("terrain/" + data)))
            {
                head = (const head_s *) (buff);
                page = (const page_s *) (buff + sizeof (head_s));
                vert = (const vert_s *) (buff + sizeof (head_s)
                                              + sizeof (page_s) * head->c);

                ::data->free(data);
            }
        }
    }

    // Initialize the transformation.

    double xs = head ? s / head->m : 1.0;
    double zs =        h / 65536.0;
/*
    load_idt(M);
    load_idt(I);
*/

    load_rot_mat(M, 1, 0, 0, -90);
    Rmul_scl_mat(M, xs, xs, zs);
}

ogl::terrain::~terrain()
{
//  if (!data.empty()) ::data->free(data);
}

//-----------------------------------------------------------------------------

ogl::range ogl::terrain::view(const double *V, int n) const
{
    return ogl::range();
}

//-----------------------------------------------------------------------------

void ogl::terrain::draw(const double *p,
                        const double *V, int n) const
{
    glUseProgramObjectARB(0);

    glPointSize(4.0);
    glColor3f(1.0f, 1.0f, 0.0);

    glPushMatrix();
    {
        glMultMatrixd(M);

        glBegin(GL_POINTS);
        {
            for (int i = 0; i < head->n * head->n; ++i)
                glVertex3sv((const GLshort *) (vert + i));
        }
        glEnd();
    }
    glPopMatrix();
}

//-----------------------------------------------------------------------------

void ogl::terrain::init()
{
}

void ogl::terrain::fini()
{
}

//-----------------------------------------------------------------------------
