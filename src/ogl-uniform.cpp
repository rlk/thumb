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
    val[0] = a;
}

void ogl::uniform::set(double a, double b)
{
    val[0] = a;
    val[1] = b;
}

void ogl::uniform::set(double a, double b, double c)
{
    val[0] = a;
    val[1] = b;
    val[2] = c;
}

void ogl::uniform::set(double a, double b, double c, double d)
{
    val[0] = a;
    val[1] = b;
    val[2] = c;
    val[3] = d;
}

void ogl::uniform::set(const double *p)
{
    for (int i = 0; i < len; ++i)
        val[i] = GLfloat(p[i]);
}

//-----------------------------------------------------------------------------

void ogl::uniform::apply(GLint location) const
{
    if (location >= 0)
        switch (len)
        {
        case  1: glUniform1fvARB(location, 1, val); break;
        case  2: glUniform2fvARB(location, 1, val); break;
        case  3: glUniform3fvARB(location, 1, val); break;
        case  4: glUniform4fvARB(location, 1, val); break;

        case  9: glUniformMatrix3fvARB(location, 1, GL_FALSE, val); break;
        case 16: glUniformMatrix4fvARB(location, 1, GL_FALSE, val); break;
        }
}

//-----------------------------------------------------------------------------
