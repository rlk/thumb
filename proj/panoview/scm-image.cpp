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
#include "scm-index.hpp"

//------------------------------------------------------------------------------

static GLuint glGetUniformLocf(GLuint program, const char *fmt, ...)
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
    glUniform1i(glGetUniformLocf(program, "%s.img", name.c_str()), unit);
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
    GLenum idx = glGetUniformLocf(program, "%s.idx[%d]", name.c_str(), d);
    GLenum age = glGetUniformLocf(program, "%s.age[%d]", name.c_str(), d);

    int u, n = cache->get_page(file, i, t, u);

    glUniform1f(idx,  GLfloat(n) / GLfloat(cache->get_size()));
    glUniform1f(age, std::min(1.f, GLfloat(t - u) / 60.f));
}

void scm_image::clr_texture(GLuint program, int d) const
{
}

void scm_image::set_uniform(GLuint program, int d, long long i) const
{
    long long r = scm_page_row(i);
    long long c = scm_page_col(i);
    long long R = r;
    long long C = c;

    for (int l = d; l >= 0; --l)
    {
        GLfloat m = 1.0f / (1 << (d - l));
        GLfloat x = m * c - C;
        GLfloat y = m * r - R;

        GLenum mul = glGetUniformLocf(program, "%s.mul[%d]", name.c_str(), l);
        GLenum add = glGetUniformLocf(program, "%s.add[%d]", name.c_str(), l);

        glUniform2f(mul, m, m);
        glUniform2f(add, x, y);

        C /= 2;
        R /= 2;
    }
}

//------------------------------------------------------------------------------
