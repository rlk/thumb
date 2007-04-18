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

#include "geodata.hpp"

//-----------------------------------------------------------------------------

ogl::geodata::geodata(std::string name) : name(name)
{
    data = new obj::obj(name);
}

ogl::geodata::~geodata()
{
    delete data;
}

//-----------------------------------------------------------------------------

void ogl::geodata::box_bound(float *b) const
{
    data->box_bound(b);
}

void ogl::geodata::sph_bound(float *b) const
{
    data->sph_bound(b);
}

void ogl::geodata::draw() const
{
    data->draw();
}

//-----------------------------------------------------------------------------
