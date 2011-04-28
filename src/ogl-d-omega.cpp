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

#include <matrix.hpp>
#include <app-conf.hpp>
#include <ogl-d-omega.hpp>

//-----------------------------------------------------------------------------

ogl::d_omega::d_omega(const std::string& name) :
    cubelut(name, ::conf->get_i("reflection_cubemap_size", 128))
{
    init();
}

ogl::d_omega::~d_omega()
{
    fini();
}

//-----------------------------------------------------------------------------

void ogl::d_omega::fill(float *p, const double *a,
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
            const double s1 = (double(j) + 1.0) / double(n);
            const double t0 = (double(i) + 0.0) / double(n);
            const double t1 = (double(i) + 1.0) / double(n);

            double A[3];
            double B[3];
            double C[3];
            double D[3];

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

            normalize(A);
            normalize(B);
            normalize(C);
            normalize(D);

            p[k] = float(angle(D, C, B, A) / (4.0 * M_PI));
        }
}

//-----------------------------------------------------------------------------
