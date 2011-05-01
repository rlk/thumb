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

#include <etc-math.hpp>
#include <app-conf.hpp>
#include <ogl-sh-basis.hpp>

//-----------------------------------------------------------------------------

ogl::sh_basis::sh_basis(const std::string& name, int i) :
    cubelut(name, ::conf->get_i("reflection_cubemap_size", 128)),

    l(int(sqrt(i))),
    m(i - l - l * l)
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

static double P(int l, int m, double x)
{
    // This function was adapted from "Spherical Harmonic Lighting:
    // The Gritty Details" by Robin Green.

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

/*
static double P(int l, int m, double x)
{
    if (l == m    ) return pow(-1, m) * factorial(factorial(2 * m - 1)) * pow(1 - x * x, m / 2.0);
    if (l == m + 1) return x * (2 * m + 1) * P(m, m, x);

    return ((2 * l - 1) * P(l - 1, m, x) * x -
            (l + m - 1) * P(l - 1, m, x)) / (l - m);
}
*/
static double Y(int l, int m, double x, double y, double z)
{
    // Not everyone agrees on the orientation of the axes or the meaning
    // of theta and phi. In the end, it doesn't matter. These make sense:

    const double phi = atan2(-z, -x);

    if (m > 0.0f) return M_SQRT2 * K(l, +m) * cos(+m * phi) * P(l, +m, y);
    if (m < 0.0f) return M_SQRT2 * K(l, -m) * sin(-m * phi) * P(l, -m, y);

    return K(l, 0) * P(l, 0, y);
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
            const double ss = (double(j) + 0.5) / double(n);
            const double tt = (double(i) + 0.5) / double(n);

            double N[3];

            N[0] = a[0] + u[0] * ss + v[0] * tt;
            N[1] = a[1] + u[1] * ss + v[1] * tt;
            N[2] = a[2] + u[2] * ss + v[2] * tt;

            normalize(N);

            p[k] = float(Y(l, m, N[0], N[1], N[2]));
        }
}

//-----------------------------------------------------------------------------
