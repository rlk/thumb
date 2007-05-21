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

ogl::surface::surface(std::string name) : name(name), data(new obj::obj(name))
{
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

GLsizei ogl::surface::count() const
{
    return data->count();
}

GLsizei ogl::surface::esize(GLsizei i) const
{
    return data->esize(i);
}

GLsizei ogl::surface::vsize(GLsizei i) const
{
    return data->vsize(i);
}

void ogl::surface::ecopy(GLsizei i, GLvoid *p, GLuint d) const
{
    data->ecopy(i, p, d);
}

void ogl::surface::vcopy(GLsizei i, GLvoid *p, const GLfloat *M) const
{
    data->vcopy(i, p, M);
}

std::string& ogl::surface::state(GLsizei i) const
{
    return data->state(i);
}

//-----------------------------------------------------------------------------
