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
#include <ogl-uniform.hpp>

//-----------------------------------------------------------------------------

ogl::uniform::uniform(std::string name, GLsizei len) : name(name), len(len)
{
    val = new GLfloat[len];
}

ogl::uniform::~uniform()
{
    delete [] val;
}

//-----------------------------------------------------------------------------

void ogl::uniform::set(double a)
{
    val[0] = GLfloat(a);
}

void ogl::uniform::set(double a, double b)
{
    val[0] = GLfloat(a);
    val[1] = GLfloat(b);
}

void ogl::uniform::set(double a, double b, double c)
{
    val[0] = GLfloat(a);
    val[1] = GLfloat(b);
    val[2] = GLfloat(c);
}

void ogl::uniform::set(double a, double b, double c, double d)
{
    val[0] = GLfloat(a);
    val[1] = GLfloat(b);
    val[2] = GLfloat(c);
    val[3] = GLfloat(d);
}

void ogl::uniform::set(const double *p)
{
    for (int i = 0; i < len; ++i)
        val[i] = GLfloat(p[i]);
}

//-----------------------------------------------------------------------------

void ogl::uniform::set(const vec3& v)
{
    assert(len == 3);

    val[0] = GLfloat(v[0]);
    val[1] = GLfloat(v[1]);
    val[2] = GLfloat(v[2]);
}

void ogl::uniform::set(const vec4& v)
{
    assert(len == 4);

    val[0] = GLfloat(v[0]);
    val[1] = GLfloat(v[1]);
    val[2] = GLfloat(v[2]);
    val[3] = GLfloat(v[3]);
}

void ogl::uniform::set(const mat4& M)
{
    assert(len == 16);

    val[ 0] = GLfloat(M[0][0]);
    val[ 1] = GLfloat(M[1][0]);
    val[ 2] = GLfloat(M[2][0]);
    val[ 3] = GLfloat(M[3][0]);
    val[ 4] = GLfloat(M[0][1]);
    val[ 5] = GLfloat(M[1][1]);
    val[ 6] = GLfloat(M[2][1]);
    val[ 7] = GLfloat(M[3][1]);
    val[ 8] = GLfloat(M[0][2]);
    val[ 9] = GLfloat(M[1][2]);
    val[10] = GLfloat(M[2][2]);
    val[11] = GLfloat(M[3][2]);
    val[12] = GLfloat(M[0][3]);
    val[13] = GLfloat(M[1][3]);
    val[14] = GLfloat(M[2][3]);
    val[15] = GLfloat(M[3][3]);
}

//-----------------------------------------------------------------------------

void ogl::uniform::apply(GLint location) const
{
    if (location >= 0)
        switch (len)
        {
        case  1: glUniform1fv(location, 1, val); break;
        case  2: glUniform2fv(location, 1, val); break;
        case  3: glUniform3fv(location, 1, val); break;
        case  4: glUniform4fv(location, 1, val); break;

        case  9: glUniformMatrix3fv(location, 1, GL_FALSE, val); break;
        case 16: glUniformMatrix4fv(location, 1, GL_FALSE, val); break;
        }
}

//-----------------------------------------------------------------------------
