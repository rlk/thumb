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

#include <iostream>
#include "surface.hpp"

//-----------------------------------------------------------------------------

ogl::surface::surface(std::string name) : name(name)
{
    init();
}

ogl::surface::~surface()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::surface::box_bound(float *b) const
{
    data->box_bound(b);
}

void ogl::surface::sph_bound(float *b) const
{
    data->sph_bound(b);
}

//-----------------------------------------------------------------------------

GLsizei ogl::surface::ecopy(GLsizei esz, GLsizei vsz)
{
    return data->ecopy(esz, vsz);
}

GLsizei ogl::surface::vcopy(GLsizei off)
{
    return data->vcopy(off);
}

GLsizei ogl::surface::esize() const
{
    return data->esize();
}

GLsizei ogl::surface::vsize() const
{
    return data->vsize();
}

//-----------------------------------------------------------------------------

void ogl::surface::draw(int type) const
{
    data->draw(type);
}

int ogl::surface::type() const
{
    return data->type();
}

//-----------------------------------------------------------------------------

void ogl::surface::init()
{
    data = new obj::obj(name);
}

void ogl::surface::fini()
{
    delete data;
}

//-----------------------------------------------------------------------------
