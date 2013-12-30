//  Copyright (C) 2009-2011 Robert Kooima
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

#include <cassert>

#include <app-conf.hpp>
#include <app-glob.hpp>
#include <ogl-texture.hpp>
#include <ogl-binding.hpp>
#include <ogl-cookie.hpp>

//-----------------------------------------------------------------------------

ogl::cookie::cookie(const std::string& name) : process(name), texture(0)
{
}

ogl::cookie::~cookie()
{
}

//-----------------------------------------------------------------------------

void ogl::cookie::draw(const ogl::binding *binding)
{
	if (binding == 0)
		texture = 0;
	else
		texture = binding->get_default_texture();
}

void ogl::cookie::bind(GLenum unit) const
{
    if (texture)
        texture->bind(unit);
}

//-----------------------------------------------------------------------------
