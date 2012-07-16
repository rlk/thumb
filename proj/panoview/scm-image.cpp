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

#include "scm-image.hpp"

//------------------------------------------------------------------------------

scm_image::scm_image(const std::string& name,
					 const std::string& scm, scm_cache *cache, int chan) :
	name(name),
	cache(cache),
	chan(chan)
{
	file = cache->add_file(scm);
}

//------------------------------------------------------------------------------
