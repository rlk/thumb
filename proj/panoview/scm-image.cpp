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

static GLuint glGetUniformLocationf(GLuint program, const char *fmt, ...)
{
    GLuint loc = 0;
    va_list ap;

    va_start(ap, fmt);
    {
        char str[256];
        vsprintf(str, fmt, ap);
        loc = glGetUniformLocation(program, str);
    }
    va_end(ap);

    return loc;
}

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

void scm_image::bind(GLint unit, GLuint program) const
{
    glUniform1i(glGetUniformLocationf(program, "%s.img", name.c_str()), unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    cache->bind();
}

void scm_image::free(GLint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

//------------------------------------------------------------------------------
