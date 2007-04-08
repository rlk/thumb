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
#include "obj/obj.h"

//-----------------------------------------------------------------------------

ogl::geodata::geodata(std::string name) : name(name)
{
    desc = obj_add_file(name.c_str());
}

ogl::geodata::~geodata()
{
    obj_del_file(desc);
}

//-----------------------------------------------------------------------------

void ogl::geodata::box_bound(float *b) const
{
    obj_get_file_box(desc, b);
}

void ogl::geodata::sph_bound(float *b) const
{
    obj_get_file_sph(desc, b);
}

void ogl::geodata::draw() const
{
    obj_draw_file(desc);
}

//-----------------------------------------------------------------------------
