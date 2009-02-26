//  Copyright (C) 2009 Robert Kooima
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

#include "ogl-uniform.hpp"

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

void ogl::uniform::set(const double *p)
{
    for (int i = 0; i < len; ++i)
        val[i] = GLfloat(p[i]);
}

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
