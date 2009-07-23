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

#include "app-data.hpp"
#include "app-file.hpp"
#include "ogl-terrain.hpp"

//-----------------------------------------------------------------------------

struct ogl::page_s
{
    uint32_t sub[4];
    uint16_t min;
    uint16_t max;
    uint16_t err;
    uint16_t pad;
};

struct ogl::vert_s
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t u;
};

//-----------------------------------------------------------------------------

ogl::terrain::terrain(std::string name) :
    name(name), buff(0), count(0), page(0), vert(0)
{
    app::file file("terrain/" + name);

    if (app::node p = file.get_root().find("terrain"))
    {
        data = p.get_s("file");

        if (!data.empty())
        {
            if ((buff = (const uint8_t *) ::data->load("terrain/" + data)))
            {
                count = ((int *) buff)[0];

                page = (const page_s *) (buff + 4);
                vert = (const vert_s *) (buff + 4 + sizeof (page_s) * count);
            }
        }
    }
}

ogl::terrain::~terrain()
{
    if (!data.empty()) ::data->free(data);
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

    glBegin(GL_POINTS);
    {
        for (int i = 0; i < count; ++i)
            glVertex3sv((const GLshort *) (vert + i));
    }
    glEnd();
}

//-----------------------------------------------------------------------------

void ogl::terrain::init()
{
}

void ogl::terrain::fini()
{
}

//-----------------------------------------------------------------------------
