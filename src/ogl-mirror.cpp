//  Copyright (C) 2010 Robert Kooima
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

#include "app-conf.hpp"
#include "app-glob.hpp"
#include "ogl-binding.hpp"
#include "ogl-mirror.hpp"

//-----------------------------------------------------------------------------

ogl::mirror::mirror(std::string name) :
    binding(::glob->load_binding(name, name)),
    frame(w, h, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, true, true, false)
{
}

ogl::mirror::~mirror()
{
    ::glob->free_binding(binding);
}

//-----------------------------------------------------------------------------

void ogl::mirror::bind() const
{
}

void ogl::mirror::free() const
{
}

void ogl::mirror::draw() const
{
}

//-----------------------------------------------------------------------------
