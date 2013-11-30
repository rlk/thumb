//  Copyright (C) 2005-2011 Robert Kooima
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

#include <etc-vector.hpp>

//-----------------------------------------------------------------------------

#define A 10
#define B 11
#define C 12
#define D 13
#define E 14
#define F 15

//-----------------------------------------------------------------------------
// Matrix operations

void load_idt(double *M)
{
    M[0] = 1; M[4] = 0; M[8] = 0; M[C] = 0;
    M[1] = 0; M[5] = 1; M[9] = 0; M[D] = 0;
    M[2] = 0; M[6] = 0; M[A] = 1; M[E] = 0;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_mat(double *M, const double *N)
{
    M[0] = N[0]; M[4] = N[4]; M[8] = N[8]; M[C] = N[C];
    M[1] = N[1]; M[5] = N[5]; M[9] = N[9]; M[D] = N[D];
    M[2] = N[2]; M[6] = N[6]; M[A] = N[A]; M[E] = N[E];
    M[3] = N[3]; M[7] = N[7]; M[B] = N[B]; M[F] = N[F];
}

void load_xps(double *M, const double *N)
{
    M[0] = N[0]; M[4] = N[1]; M[8] = N[2]; M[C] = N[3];
    M[1] = N[4]; M[5] = N[5]; M[9] = N[6]; M[D] = N[7];
    M[2] = N[8]; M[6] = N[9]; M[A] = N[A]; M[E] = N[B];
    M[3] = N[C]; M[7] = N[D]; M[B] = N[E]; M[F] = N[F];
}

void load_inv(double *I, const double *M)
{
    double T[16];
    double d;

    T[0] = +(M[5] * (M[A] * M[F] - M[B] * M[E]) -
             M[9] * (M[6] * M[F] - M[7] * M[E]) +
             M[D] * (M[6] * M[B] - M[7] * M[A]));
    T[1] = -(M[4] * (M[A] * M[F] - M[B] * M[E]) -
             M[8] * (M[6] * M[F] - M[7] * M[E]) +
             M[C] * (M[6] * M[B] - M[7] * M[A]));
    T[2] = +(M[4] * (M[9] * M[F] - M[B] * M[D]) -
             M[8] * (M[5] * M[F] - M[7] * M[D]) +
             M[C] * (M[5] * M[B] - M[7] * M[9]));
    T[3] = -(M[4] * (M[9] * M[E] - M[A] * M[D]) -
             M[8] * (M[5] * M[E] - M[6] * M[D]) +
             M[C] * (M[5] * M[A] - M[6] * M[9]));

    T[4] = -(M[1] * (M[A] * M[F] - M[B] * M[E]) -
             M[9] * (M[2] * M[F] - M[3] * M[E]) +
             M[D] * (M[2] * M[B] - M[3] * M[A]));
    T[5] = +(M[0] * (M[A] * M[F] - M[B] * M[E]) -
             M[8] * (M[2] * M[F] - M[3] * M[E]) +
             M[C] * (M[2] * M[B] - M[3] * M[A]));
    T[6] = -(M[0] * (M[9] * M[F] - M[B] * M[D]) -
             M[8] * (M[1] * M[F] - M[3] * M[D]) +
             M[C] * (M[1] * M[B] - M[3] * M[9]));
    T[7] = +(M[0] * (M[9] * M[E] - M[A] * M[D]) -
             M[8] * (M[1] * M[E] - M[2] * M[D]) +
             M[C] * (M[1] * M[A] - M[2] * M[9]));

    T[8] = +(M[1] * (M[6] * M[F] - M[7] * M[E]) -
             M[5] * (M[2] * M[F] - M[3] * M[E]) +
             M[D] * (M[2] * M[7] - M[3] * M[6]));
    T[9] = -(M[0] * (M[6] * M[F] - M[7] * M[E]) -
             M[4] * (M[2] * M[F] - M[3] * M[E]) +
             M[C] * (M[2] * M[7] - M[3] * M[6]));
    T[A] = +(M[0] * (M[5] * M[F] - M[7] * M[D]) -
             M[4] * (M[1] * M[F] - M[3] * M[D]) +
             M[C] * (M[1] * M[7] - M[3] * M[5]));
    T[B] = -(M[0] * (M[5] * M[E] - M[6] * M[D]) -
             M[4] * (M[1] * M[E] - M[2] * M[D]) +
             M[C] * (M[1] * M[6] - M[2] * M[5]));

    T[C] = -(M[1] * (M[6] * M[B] - M[7] * M[A]) -
             M[5] * (M[2] * M[B] - M[3] * M[A]) +
             M[9] * (M[2] * M[7] - M[3] * M[6]));
    T[D] = +(M[0] * (M[6] * M[B] - M[7] * M[A]) -
             M[4] * (M[2] * M[B] - M[3] * M[A]) +
             M[8] * (M[2] * M[7] - M[3] * M[6]));
    T[E] = -(M[0] * (M[5] * M[B] - M[7] * M[9]) -
             M[4] * (M[1] * M[B] - M[3] * M[9]) +
             M[8] * (M[1] * M[7] - M[3] * M[5]));
    T[F] = +(M[0] * (M[5] * M[A] - M[6] * M[9]) -
             M[4] * (M[1] * M[A] - M[2] * M[9]) +
             M[8] * (M[1] * M[6] - M[2] * M[5]));

    d = 1.0 / (M[0] * T[0] + M[4] * T[4] + M[8] * T[8] + M[C] * T[C]);

    I[0] = T[0] * d;
    I[1] = T[4] * d;
    I[2] = T[8] * d;
    I[3] = T[C] * d;
    I[4] = T[1] * d;
    I[5] = T[5] * d;
    I[6] = T[9] * d;
    I[7] = T[D] * d;
    I[8] = T[2] * d;
    I[9] = T[6] * d;
    I[A] = T[A] * d;
    I[B] = T[E] * d;
    I[C] = T[3] * d;
    I[D] = T[7] * d;
    I[E] = T[B] * d;
    I[F] = T[F] * d;
}

//-----------------------------------------------------------------------------
// Matrix constructors

void load_xlt_mat(double *M, double x, double y, double z)
{
    M[0] = 1; M[4] = 0; M[8] = 0; M[C] = x;
    M[1] = 0; M[5] = 1; M[9] = 0; M[D] = y;
    M[2] = 0; M[6] = 0; M[A] = 1; M[E] = z;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_scl_mat(double *M, double x, double y, double z)
{
    M[0] = x; M[4] = 0; M[8] = 0; M[C] = 0;
    M[1] = 0; M[5] = y; M[9] = 0; M[D] = 0;
    M[2] = 0; M[6] = 0; M[A] = z; M[E] = 0;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_rot_mat(double *M, double x, double y, double z, double a)
{
    double U[16], S[16], u[3], k = sqrt(x * x + y * y + z * z);

    const double s = sin(RAD(a));
    const double c = cos(RAD(a));

    u[0] = x / k;
    u[1] = y / k;
    u[2] = z / k;

    U[0] = u[0] * u[0]; U[4] = u[0] * u[1]; U[8] = u[0] * u[2];
    U[1] = u[1] * u[0]; U[5] = u[1] * u[1]; U[9] = u[1] * u[2];
    U[2] = u[2] * u[0]; U[6] = u[2] * u[1]; U[A] = u[2] * u[2];

    S[0] =     0; S[4] = -u[2]; S[8] =  u[1];
    S[1] =  u[2]; S[5] =     0; S[9] = -u[0];
    S[2] = -u[1]; S[6] =  u[0]; S[A] =     0;

    M[0] = U[0] + c * (1 - U[0]) + s * S[0];
    M[1] = U[1] + c * (0 - U[1]) + s * S[1];
    M[2] = U[2] + c * (0 - U[2]) + s * S[2];
    M[3] = 0;
    M[4] = U[4] + c * (0 - U[4]) + s * S[4];
    M[5] = U[5] + c * (1 - U[5]) + s * S[5];
    M[6] = U[6] + c * (0 - U[6]) + s * S[6];
    M[7] = 0;
    M[8] = U[8] + c * (0 - U[8]) + s * S[8];
    M[9] = U[9] + c * (0 - U[9]) + s * S[9];
    M[A] = U[A] + c * (1 - U[A]) + s * S[A];
    M[B] = 0;
    M[C] = 0;
    M[D] = 0;
    M[E] = 0;
    M[F] = 1;
}

//-----------------------------------------------------------------------------
// Projection constructors

void load_persp(double *P, double l, double r,
                           double b, double t,
                           double n, double f)
{
    P[0] =  (2 * n) / (r - l);
    P[1] =  0;
    P[2] =  0;
    P[3] =  0;
    P[4] =  0;
    P[5] =  (2 * n) / (t - b);
    P[6] =  0;
    P[7] =  0;
    P[8] =  (r + l) / (r - l);
    P[9] =  (t + b) / (t - b);
    P[A] = -(f + n) / (f - n);
    P[B] = -1;
    P[C] =  0;
    P[D] =  0;
    P[E] = -2 * f * n / (f - n);
    P[F] =  0;
}

void load_ortho(double *P, double l, double r,
                           double b, double t,
                           double n, double f)
{
    P[0] =  2 / (r - l);
    P[1] =  0;
    P[2] =  0;
    P[3] =  0;
    P[4] =  0;
    P[5] =  2 / (t - b);
    P[6] =  0;
    P[7] =  0;
    P[8] =  0;
    P[9] =  0;
    P[A] = -2 / (f - n);
    P[B] =  0;
    P[C] = -(r + l) / (r - l);
    P[D] = -(t + b) / (t - b);
    P[E] = -(f + n) / (f - n);
    P[F] =  1;
}

//-----------------------------------------------------------------------------
// Matrix inverse constructors

void load_xlt_inv(double *I, double x, double y, double z)
{
    load_xlt_mat(I, -x, -y, -z);
}

void load_scl_inv(double *I, double x, double y, double z)
{
    load_scl_mat(I, 1 / x, 1 / y, 1 / z);
}

void load_rot_inv(double *I, double x, double y, double z, double a)
{
    load_rot_mat(I, x, y, z, -a);
}

//-----------------------------------------------------------------------------
// Matrix left-composers

void Lmul_xlt_mat(double *M, double x, double y, double z)
{
    double T[16];

    load_xlt_mat(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_scl_mat(double *M, double x, double y, double z)
{
    double T[16];

    load_scl_mat(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_rot_mat(double *M, double x, double y, double z, double a)
{
    double T[16];

    load_rot_mat(T, x, y, z, a);
    mult_mat_mat(M, T, M);
}

//-----------------------------------------------------------------------------
// Matrix inverse left-composers

void Lmul_xlt_inv(double *M, double x, double y, double z)
{
    double T[16];

    load_xlt_inv(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_scl_inv(double *M, double x, double y, double z)
{
    double T[16];

    load_scl_inv(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_rot_inv(double *M, double x, double y, double z, double a)
{
    double T[16];

    load_rot_inv(T, x, y, z, a);
    mult_mat_mat(M, T, M);
}

//-----------------------------------------------------------------------------
// Matrix right-composers

void Rmul_xlt_mat(double *M, double x, double y, double z)
{
    double T[16];

    load_xlt_mat(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_scl_mat(double *M, double x, double y, double z)
{
    double T[16];

    load_scl_mat(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_rot_mat(double *M, double x, double y, double z, double a)
{
    double T[16];

    load_rot_mat(T, x, y, z, a);
    mult_mat_mat(M, M, T);
}

//-----------------------------------------------------------------------------
// Matrix inverse right-composers

void Rmul_xlt_inv(double *M, double x, double y, double z)
{
    double T[16];

    load_xlt_inv(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_scl_inv(double *M, double x, double y, double z)
{
    double T[16];

    load_scl_inv(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_rot_inv(double *M, double x, double y, double z, double a)
{
    double T[16];

    load_rot_inv(T, x, y, z, a);
    mult_mat_mat(M, M, T);
}

//-----------------------------------------------------------------------------
// Multipliers and transformers

void mult_mat_mat(double *M, const double N[16], const double O[16])
{
    double T[16];

    T[0] = N[0] * O[0] + N[4] * O[1] + N[8] * O[2] + N[C] * O[3];
    T[1] = N[1] * O[0] + N[5] * O[1] + N[9] * O[2] + N[D] * O[3];
    T[2] = N[2] * O[0] + N[6] * O[1] + N[A] * O[2] + N[E] * O[3];
    T[3] = N[3] * O[0] + N[7] * O[1] + N[B] * O[2] + N[F] * O[3];

    T[4] = N[0] * O[4] + N[4] * O[5] + N[8] * O[6] + N[C] * O[7];
    T[5] = N[1] * O[4] + N[5] * O[5] + N[9] * O[6] + N[D] * O[7];
    T[6] = N[2] * O[4] + N[6] * O[5] + N[A] * O[6] + N[E] * O[7];
    T[7] = N[3] * O[4] + N[7] * O[5] + N[B] * O[6] + N[F] * O[7];

    T[8] = N[0] * O[8] + N[4] * O[9] + N[8] * O[A] + N[C] * O[B];
    T[9] = N[1] * O[8] + N[5] * O[9] + N[9] * O[A] + N[D] * O[B];
    T[A] = N[2] * O[8] + N[6] * O[9] + N[A] * O[A] + N[E] * O[B];
    T[B] = N[3] * O[8] + N[7] * O[9] + N[B] * O[A] + N[F] * O[B];

    T[C] = N[0] * O[C] + N[4] * O[D] + N[8] * O[E] + N[C] * O[F];
    T[D] = N[1] * O[C] + N[5] * O[D] + N[9] * O[E] + N[D] * O[F];
    T[E] = N[2] * O[C] + N[6] * O[D] + N[A] * O[E] + N[E] * O[F];
    T[F] = N[3] * O[C] + N[7] * O[D] + N[B] * O[E] + N[F] * O[F];

    load_mat(M, T);
}

void mult_mat_vec3(double *v, const double *M, const double *u)
{
    v[0] = M[0] * u[0] + M[4] * u[1] + M[8] * u[2] + M[C];
    v[1] = M[1] * u[0] + M[5] * u[1] + M[9] * u[2] + M[D];
    v[2] = M[2] * u[0] + M[6] * u[1] + M[A] * u[2] + M[E];
}

void mult_xps_vec3(double *v, const double *M, const double *u)
{
    v[0] = M[0] * u[0] + M[1] * u[1] + M[2] * u[2] + M[3];
    v[1] = M[4] * u[0] + M[5] * u[1] + M[6] * u[2] + M[7];
    v[2] = M[8] * u[0] + M[9] * u[1] + M[A] * u[2] + M[B];
}

void mult_mat_vec4(double *v, const double *M, const double *u)
{
    v[0] = M[0] * u[0] + M[4] * u[1] + M[8] * u[2] + M[C] * u[3];
    v[1] = M[1] * u[0] + M[5] * u[1] + M[9] * u[2] + M[D] * u[3];
    v[2] = M[2] * u[0] + M[6] * u[1] + M[A] * u[2] + M[E] * u[3];
    v[3] = M[3] * u[0] + M[7] * u[1] + M[B] * u[2] + M[F] * u[3];
}

void mult_xps_vec4(double *v, const double *M, const double *u)
{
    v[0] = M[0] * u[0] + M[1] * u[1] + M[2] * u[2] + M[3] * u[3];
    v[1] = M[4] * u[0] + M[5] * u[1] + M[6] * u[2] + M[7] * u[3];
    v[2] = M[8] * u[0] + M[9] * u[1] + M[A] * u[2] + M[B] * u[3];
    v[3] = M[C] * u[0] + M[D] * u[1] + M[E] * u[2] + M[F] * u[3];
}

//-----------------------------------------------------------------------------

void midpoint(double *m, const double *a, const double *b)
{
    double bx = cos(b[1]) * cos(b[0] - a[0]);
    double by = cos(b[1]) * sin(b[0] - a[0]);

    double dx = cos(a[1]) + bx;

    m[0] = a[0] + atan2(by, dx);

    m[1] = atan2(sin(a[1]) + sin(b[1]), sqrt(dx * dx + by * by));
}

double distance(const double *a, const double *b)
{
    double d[3];

    d[0] = a[0] - b[0];
    d[1] = a[1] - b[1];
    d[2] = a[2] - b[2];

    return sqrt(DOT3(d, d));
}

//-----------------------------------------------------------------------------
// Quaternion / matrix conversions

void mat_to_quat(double *q, const double *M)
{
    if (1.0 + M[0] + M[5] + M[10] > 0.001)
    {
        double s = 0.5 / sqrt(1.0 + M[0] + M[5] + M[10]);

        q[0] = (M[6] - M[9]) * s;
        q[1] = (M[8] - M[2]) * s;
        q[2] = (M[1] - M[4]) * s;
        q[3] =         0.25  / s;
    }
    else if (M[0] > M[5] && M[0] > M[10])
    {
        double s = 2.0 * sqrt(1.0 + M[0] - M[5] - M[10]);

        q[0] =         0.25  * s;
        q[1] = (M[1] + M[4]) / s;
        q[2] = (M[8] + M[2]) / s;
        q[3] = (M[6] - M[9]) / s;
    }
    else if (M[5] > M[10])
    {
        double s = 2.0 * sqrt(1.0 + M[5] - M[0] - M[10]);

        q[0] = (M[1] + M[4]) / s;
        q[1] =         0.25  * s;
        q[2] = (M[6] + M[9]) / s;
        q[3] = (M[8] - M[2]) / s;
    }
    else
    {
        double s = 2.0 * sqrt(1.0 + M[10] - M[0] - M[5]);

        q[0] = (M[8] + M[2]) / s;
        q[1] = (M[6] + M[9]) / s;
        q[2] =         0.25  * s;
        q[3] = (M[1] - M[4]) / s;
    }
}

void quat_to_mat(double *M, const double *q)
{
    M[ 0] = 1 - 2 * (q[1] * q[1] + q[2] * q[2]);
    M[ 1] =     2 * (q[0] * q[1] + q[2] * q[3]);
    M[ 2] =     2 * (q[0] * q[2] - q[1] * q[3]);
    M[ 3] = 0;

    M[ 4] =     2 * (q[0] * q[1] - q[2] * q[3]);
    M[ 5] = 1 - 2 * (q[0] * q[0] + q[2] * q[2]);
    M[ 6] =     2 * (q[1] * q[2] + q[0] * q[3]);
    M[ 7] = 0;

    M[ 8] =     2 * (q[0] * q[2] + q[1] * q[3]);
    M[ 9] =     2 * (q[1] * q[2] - q[0] * q[3]);
    M[10] = 1 - 2 * (q[0] * q[0] + q[1] * q[1]);
    M[11] = 0;

    M[12] = 0;
    M[13] = 0;
    M[14] = 0;
    M[15] = 1;
}

void quat_slerp(double *q, const double *a, const double *b, double t)
{
    const double d = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
    const double k = acos(fabs(d));

    const double u = sin(k - t * k) / sin(k);
    const double v = sin(    t * k) / sin(k);

    if (fabs(d) < 1.0)
    {
        if (d > 0.0)
        {
            q[0] = a[0] * u + b[0] * v;
            q[1] = a[1] * u + b[1] * v;
            q[2] = a[2] * u + b[2] * v;
            q[3] = a[3] * u + b[3] * v;
        }
        else
        {
            q[0] = a[0] * u - b[0] * v;
            q[1] = a[1] * u - b[1] * v;
            q[2] = a[2] * u - b[2] * v;
            q[3] = a[3] * u - b[3] * v;
        }
    }
    else
    {
        q[0] = a[0];
        q[1] = a[1];
        q[2] = a[2];
        q[3] = a[3];
    }
}

void quat_mult(double *a, const double *b, const double *c)
{
    double t[4];

    t[0] = b[0] * c[3] + b[3] * c[0] + b[1] * c[2] - b[2] * c[1];
    t[1] = b[1] * c[3] + b[3] * c[1] + b[2] * c[0] - b[0] * c[2];
    t[2] = b[2] * c[3] + b[3] * c[2] + b[0] * c[1] - b[1] * c[0];
    t[3] = b[3] * c[3] - b[0] * c[0] - b[1] * c[1] - b[2] * c[2];

    const double d = sqrt(t[0] * t[0] + t[1] * t[1] + t[2] * t[2] + t[3] * t[3]);

    a[0] = t[0] / d;
    a[1] = t[1] / d;
    a[2] = t[2] / d;
    a[3] = t[3] / d;
}

void quat_inv(double *a, const double *b)
{
    const double d = b[0] * b[0] + b[1] * b[1] + b[2] * b[2] + b[3] * b[3];

    a[0] = -b[0] / d;
    a[1] = -b[1] / d;
    a[2] = -b[2] / d;
    a[3] =  b[3] / d;
}

void orthonormalize(double *M)
{
    normalize(M + 8);
    crossprod(M + 4, M + 8, M + 0);
    normalize(M + 4);
    crossprod(M + 0, M + 4, M + 8);
    normalize(M + 0);
}

//-----------------------------------------------------------------------------

void set_basis(double *M, const double *x, const double *y, const double *z)
{
    M[0] = x[0]; M[4] = y[0]; M[ 8] = z[0]; M[12] =  0.0;
    M[1] = x[1]; M[5] = y[1]; M[ 9] = z[1]; M[13] =  0.0;
    M[2] = x[2]; M[6] = y[2]; M[10] = z[2]; M[14] =  0.0;
    M[3] =  0.0; M[7] =  0.0; M[11] =  0.0; M[15] =  1.0;
}

double solid_angle(const double *a, const double *b, const double *c)
{
    double t[4];

    crossprod(t, b, c);

    double n = DOT3(a, t);

    double la = sqrt(DOT3(a, a));
    double lb = sqrt(DOT3(b, b));
    double lc = sqrt(DOT3(c, c));

    double d = la * lb * lc + (DOT3(a, b) * lc +
                               DOT3(a, c) * lb +
                               DOT3(b, c) * la);

    return 2.0 * atan(n / d);
}

//-----------------------------------------------------------------------------

void sphere_to_vector(double *v, double t, double p, double r)
{
    v[0] = r * sin(t) * cos(p);
    v[1] = r *          sin(p);
    v[2] = r * cos(t) * cos(p);
}

void vector_to_sphere(double *r, double x, double y, double z)
{
    double d = sqrt(x * x + y * y + z * z);

    r[0] = atan2(x, z);
    r[1] = asin(y / d);
    r[2] = d;
}

//-----------------------------------------------------------------------------

void slerp(double *p, const double *a, const double *b, double t)
{
    // Compute vector lengths and normalized arc end-points.

    double al = sqrt(DOT3(a, a));
    double bl = sqrt(DOT3(b, b));

    double an[3];
    double bn[3];

    an[0] = a[0] / al;
    an[1] = a[1] / al;
    an[2] = a[2] / al;

    bn[0] = b[0] / bl;
    bn[1] = b[1] / bl;
    bn[2] = b[2] / bl;

    // Compute the spherical linear interpolation of the arc end-points.

    const double omega = acos(DOT3(an, bn));

    if (omega > 0.001)
    {
        const double ka = sin((1 - t) * omega) / sin(omega);
        const double kb = sin((    t) * omega) / sin(omega);

        p[0] = ka * an[0] + kb * bn[0];
        p[1] = ka * an[1] + kb * bn[1];
        p[2] = ka * an[2] + kb * bn[2];
    }
    else
    {
        p[0] = an[0];
        p[1] = an[1];
        p[2] = an[2];
    }

    // Compute the linear interpolation of the vector lengths.

    double pl = (1 - t) * al + t * bl;

    p[0] *= pl;
    p[1] *= pl;
    p[2] *= pl;
}

//-----------------------------------------------------------------------------

// Round to the nearest integer.  Round 0.5 toward negative infinity.

int nearest_int(double d)
{
    double f = floor(d);
    double c = ceil (d);

    return int((c - d < d - f) ? c : f);
}

// Return a power of two greater than or equal to n.

int next_pow2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

//-----------------------------------------------------------------------------
