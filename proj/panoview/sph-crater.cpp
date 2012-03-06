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

#include <ogl-opengl.hpp>
#include <app-file.hpp>

#include "sph-crater.hpp"

//------------------------------------------------------------------------------

sph_crater::sph_crater(const std::string& dat)
{
	app::file file(dat);

	if (app::node p = file.get_root().find("planet"))
	{
        for (app::node c = p.find("crater"); c; c = p.next(c, "crater"))
        {
        	const std::string& name = c.get_s("name");
        	float              d    = c.get_f("diameter");
        	float              lat  = c.get_f("lat");
        	float              lon  = c.get_f("lon");

        	printf("%s\n", name.c_str());
        }
	}
}

sph_crater::~sph_crater()
{
}

void sph_crater::draw(const double *p)
{
}

//------------------------------------------------------------------------------
