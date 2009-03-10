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

static double factorial(int i)
{
    static const double f[] = {
        1.,
        1.,
        2.,
        6.,
        24.,
        120.,
        720.,
        5040.,
        40320.,
        362880.,
        3.6288e6,
        3.99168e7,
        4.790016e8,
        6.2270208e9,
        8.71782912e10,
        1.307674368e12,
        2.0922789888e13,
        3.55687428096e14,
        6.402373705728e15,
        1.21645100408832e17,
        2.43290200817664e18,
        5.109094217170944e19,
        1.124000727777607e21,
        2.585201673888498e22,
        6.204484017332394e23,
        1.551121004333098e25,
        4.032914611266056e26,
        1.088886945041835e28,
        3.048883446117138e29,
        8.841761993739702e30,
        2.652528598121910e32,
        8.222838654177922e33,
        2.631308369336935e35,
        8.683317618811886e36
    };
    return f[i];
}


static double K(int l, int m)
{
    const double n = (2 * l + 1)  * factorial(l - m);
    const double d = (4.0 * M_PI) * factorial(l + m);

    return sqrt(n / d);
}
/*
static double P(int l, int m, double x)
{
    double pmm = 1.0; 
    double pll = 0.0; 
    
    if (m > 0)
    { 
        double s = sqrt((1.0 - x) * (1.0 + x)); 
        double f = 1.0; 

        for(int i = 1; i <= m; ++i)
        { 
            pmm *= s * (-f); 
            f   += 2.0; 
        } 
    } 

    if (l == m)    return pmm; 

    double pm1 = x * (2.0 * m + 1.0) * pmm; 

    if (l == m + 1) return pm1;

    for(int ll = m + 2; ll <= l; ++ll)
    { 
        pll = ((2 * ll - 1) * pm1 * x -
               (m + ll - 1) * pmm) / (ll - m); 

        pmm = pm1; 
        pm1 = pll; 
    } 

    return pll; 
}
*/

static double P(int l, int m, double x)
{
    if (l == m    ) return pow(-1, m) * factorial(factorial(2 * m - 1)) * pow(1 - x * x, m / 2.0);
    if (l == m + 1) return x * (2 * m + 1) * P(m, m, x);

    return ((2 * l - 1) * P(l - 1, m, x) * x -
            (l + m - 1) * P(l - 1, m, x)) / (l - m);
}

static double Y(int l, int m, double x, double y, double z)
{
    // Not everyone agrees on the orientation of the axes or the meaning
    // of theta and phi. In the end, it doesn't matter. These make sense:

    const double phi = atan2(-z, -x);

    if (m > 0.0f) return M_SQRT2 * K(l, +m) * cos(+m * phi) * P(l, +m, y);
    if (m < 0.0f) return M_SQRT2 * K(l, -m) * sin(-m * phi) * P(l, -m, y);

    return K(l, 0) * P(l, 0, y);
}

static double domega(const double *a,
                     const double *b,
                     const double *c,
                     const double *d)
{
    double ab = DOT3(a, b);
    double ad = DOT3(a, d);
    double bc = DOT3(b, c);
    double bd = DOT3(b, d);
    double cd = DOT3(c, d);

    double bcd[3];
    double dcb[3];

    crossprod(bcd, b, d);
    crossprod(dcb, d, b);

    return 2.0 * (atan2(DOT3(a, bcd), 1.0 + ab + ad + bd) +
                  atan2(DOT3(c, dcb), 1.0 + bc + cd + bd));
}

//-----------------------------------------------------------------------------

void ogl::sh_basis::fill(float *p, const double *a,
                                   const double *b,
                                   const double *c,
                                   const double *d) const
{
    double u[3];
    double v[3];

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
            const double s0 = (double(j) + 0.0) / double(n);
            const double ss = (double(j) + 0.5) / double(n);
            const double s1 = (double(j) + 1.0) / double(n);
            const double t0 = (double(i) + 0.0) / double(n);
            const double tt = (double(i) + 0.5) / double(n);
            const double t1 = (double(i) + 1.0) / double(n);

            double N[3];
            double A[3];
            double B[3];
            double C[3];
            double D[3];

            N[0] = a[0] + u[0] * ss + v[0] * tt;
            N[1] = a[1] + u[1] * ss + v[1] * tt;
            N[2] = a[2] + u[2] * ss + v[2] * tt;

            A[0] = a[0] + u[0] * s0 + v[0] * t0;
            A[1] = a[1] + u[1] * s0 + v[1] * t0;
            A[2] = a[2] + u[2] * s0 + v[2] * t0;

            B[0] = a[0] + u[0] * s1 + v[0] * t0;
            B[1] = a[1] + u[1] * s1 + v[1] * t0;
            B[2] = a[2] + u[2] * s1 + v[2] * t0;

            C[0] = a[0] + u[0] * s1 + v[0] * t1;
            C[1] = a[1] + u[1] * s1 + v[1] * t1;
            C[2] = a[2] + u[2] * s1 + v[2] * t1;

            D[0] = a[0] + u[0] * s0 + v[0] * t1;
            D[1] = a[1] + u[1] * s0 + v[1] * t1;
            D[2] = a[2] + u[2] * s0 + v[2] * t1;

            normalize(N);
            normalize(A);
            normalize(B);
            normalize(C);
            normalize(D);

            p[k + 0] = float(Y(l, m, N[0], N[1], N[2]) * domega(A, B, C, D));
        }
}

void ogl::sh_basis::init()
{
    assert(object == 0);

    static const double v[8][3] = {
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
