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
#include <cmath>

#include <etc-vector.hpp>
#include <ogl-cubelut.hpp>

//-----------------------------------------------------------------------------

ogl::cubelut::cubelut(const std::string& name, int n) :
    process(name), n(n), object(0)
{
}

//-----------------------------------------------------------------------------

double ogl::cubelut::angle(const vec3 &a,
                           const vec3 &b,
                           const vec3 &c,
                           const vec3 &d) const
{
    double ab = a * b;
    double ad = a * d;
    double bc = b * c;
    double bd = b * d;
    double cd = c * d;

    vec3 bcd = cross(b, d);
    vec3 dcb = cross(d, b);

    return 2.0 * (atan2(a * bcd, 1.0 + ab + ad + bd) +
                  atan2(c * dcb, 1.0 + bc + cd + bd));
}

//-----------------------------------------------------------------------------

void ogl::cubelut::init()
{
    assert(object == 0);

    vec3 v[8];

    v[0] = vec3(-1, -1, -1);
    v[1] = vec3( 1, -1, -1);
    v[2] = vec3(-1,  1, -1);
    v[3] = vec3( 1,  1, -1);
    v[4] = vec3(-1, -1,  1);
    v[5] = vec3( 1, -1,  1);
    v[6] = vec3(-1,  1,  1);
    v[7] = vec3( 1,  1,  1);

    const GLenum  i = GL_LUMINANCE32F_ARB;
    const GLenum  e = GL_LUMINANCE;
    const GLenum  t = GL_FLOAT;

    const GLsizei b = 1;
    const GLsizei w = n + 2 * b;
    const GLsizei h = n + 2 * b;

    glGenTextures(1, &object);

    ogl::bind_texture(GL_TEXTURE_CUBE_MAP, GL_TEXTURE0, object);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);

    float *p = new float[w * h];

    fill(p, v[7], v[3], v[1], v[5]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, i, w, h, b, e, t, p);

    fill(p, v[2], v[6], v[4], v[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, i, w, h, b, e, t, p);

    fill(p, v[2], v[3], v[7], v[6]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, i, w, h, b, e, t, p);

    fill(p, v[4], v[5], v[1], v[0]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, i, w, h, b, e, t, p);

    fill(p, v[6], v[7], v[5], v[4]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, i, w, h, b, e, t, p);

    fill(p, v[3], v[2], v[0], v[1]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, i, w, h, b, e, t, p);

    delete [] p;
}

void ogl::cubelut::fini()
{
    assert(object);

    glDeleteTextures(1, &object);
    object = 0;
}

//-----------------------------------------------------------------------------

void ogl::cubelut::bind(GLenum unit) const
{
    assert(object);

    ogl::bind_texture(GL_TEXTURE_CUBE_MAP, unit, object);
}

//-----------------------------------------------------------------------------
