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

#include "glsl.h"

#include "scm-image.hpp"
#include "scm-index.hpp"

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
    glUniform1i(glsl_uniform(program, "%s.img", name.c_str()), unit);
    glActiveTexture(GL_TEXTURE0 + unit);
    cache->bind();
}

void scm_image::free(GLint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

//------------------------------------------------------------------------------

void scm_image::set_texture(GLuint program, int d, int t, long long i) const
{
    GLint idx = glsl_uniform(program, "%s.idx[%d]", name.c_str(), d);
    GLint age = glsl_uniform(program, "%s.age[%d]", name.c_str(), d);

    int u, n = cache->get_page(file, i, t, u);

    glUniform1f(idx,  GLfloat(n));
//    glUniform1f(idx,  GLfloat(n) / GLfloat(cache->get_size()));
    glUniform1f(age, std::min(1.f, GLfloat(t - u) / 60.f));
}

void scm_image::clr_texture(GLuint program, int d) const
{
    GLint idx = glsl_uniform(program, "%s.idx[%d]", name.c_str(), d);
    GLint age = glsl_uniform(program, "%s.age[%d]", name.c_str(), d);

    glUniform1f(idx, 0.0);
    glUniform1f(age, 0.0);
}

//------------------------------------------------------------------------------
