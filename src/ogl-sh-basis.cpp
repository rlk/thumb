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

#include <cassert>
#include <cmath>

#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-conf.hpp"
#include "ogl-frame.hpp"
#include "ogl-program.hpp"
#include "ogl-sh-basis.hpp"

//-----------------------------------------------------------------------------

ogl::sh_basis::sh_basis(const std::string& name, int i) :
    process(name),

    n(::conf->get_i("reflection_cubemap_size", 128)),
    l(int(sqrt(i))),
    m(i - l - l * l),

    object(0)
{
    init();
}

ogl::sh_basis::~sh_basis()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::sh_basis::fill(float *p, const float *a,
                                   const float *b,
                                   const float *c,
                                   const float *d) const
{
    float u[3];
    float v[3];
    float N[3];

    u[0] = b[0] - a[0];
    u[1] = b[1] - a[1];
    u[2] = b[2] - a[2];

    v[0] = d[0] - a[0];
    v[1] = d[1] - a[1];
    v[2] = d[2] - a[2];

    int i;
    int j;
    int k;

    for (k = 0, i = -1; i <= n; ++i)
        for (j = -1; j <= n; ++j, ++k)
        {
            float s = (float(j) + 0.5f) / float(n);
            float t = (float(i) + 0.5f) / float(n);

            N[0] = a[0] + u[0] * s + v[0] * t;
            N[1] = a[1] + u[1] * s + v[1] * t;
            N[2] = a[2] + u[2] * s + v[2] * t;

            normalize(N);

            p[k + 0] = N[1];
        }
}

void ogl::sh_basis::init()
{
    assert(object == 0);

    static const float v[8][3] = {
        { -1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f, -1.0f },
        { -1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f, -1.0f },
        { -1.0f, -1.0f,  1.0f },
        {  1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f,  1.0f },
        {  1.0f,  1.0f,  1.0f } 
    };
    const GLenum  i = GL_LUMINANCE32F_ARB;
    const GLenum  e = GL_LUMINANCE;
    const GLenum  t = GL_FLOAT;

    const GLsizei b = 1;
    const GLsizei w = n + 2 * b;
    const GLsizei h = n + 2 * b;

    glGenTextures(1, &object);

    ogl::bind_texture(GL_TEXTURE_CUBE_MAP, 0, object);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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

    OGLCK();
}

void ogl::sh_basis::fini()
{
    assert(object);

    glDeleteTextures(1, &object);
    object = 0;

    OGLCK();
}

//-----------------------------------------------------------------------------

void ogl::sh_basis::bind(GLenum unit) const
{
    assert(object);

    ogl::bind_texture(GL_TEXTURE_CUBE_MAP, unit, object);
}

//-----------------------------------------------------------------------------
