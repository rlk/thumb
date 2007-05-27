//  Copyright (C) 2005 Robert Kooima
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

#include "util.hpp"
#include "matrix.hpp"

//-----------------------------------------------------------------------------

#define A 10
#define B 11
#define C 12
#define D 13
#define E 14
#define F 15

//-----------------------------------------------------------------------------
// Matrix operations

void load_idt(float M[16])
{
    M[0] = 1; M[4] = 0; M[8] = 0; M[C] = 0;
    M[1] = 0; M[5] = 1; M[9] = 0; M[D] = 0;
    M[2] = 0; M[6] = 0; M[A] = 1; M[E] = 0;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_mat(float M[16], const float N[16])
{
    M[0] = N[0]; M[4] = N[4]; M[8] = N[8]; M[C] = N[C];
    M[1] = N[1]; M[5] = N[5]; M[9] = N[9]; M[D] = N[D];
    M[2] = N[2]; M[6] = N[6]; M[A] = N[A]; M[E] = N[E];
    M[3] = N[3]; M[7] = N[7]; M[B] = N[B]; M[F] = N[F];
}

void load_xps(float M[16], const float N[16])
{
    M[0] = N[0]; M[4] = N[1]; M[8] = N[2]; M[C] = N[3];
    M[1] = N[4]; M[5] = N[5]; M[9] = N[6]; M[D] = N[7];
    M[2] = N[8]; M[6] = N[9]; M[A] = N[A]; M[E] = N[B];
    M[3] = N[C]; M[7] = N[D]; M[B] = N[E]; M[F] = N[F];
}

void load_inv(float I[16], const float M[16])
{
    float T[16];
    float d;

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

    d = 1.0f / (M[0] * T[0] + M[4] * T[4] + M[8] * T[8] + M[C] * T[C]);

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

void load_xlt_mat(float M[16], float x, float y, float z)
{
    M[0] = 1; M[4] = 0; M[8] = 0; M[C] = x;
    M[1] = 0; M[5] = 1; M[9] = 0; M[D] = y;
    M[2] = 0; M[6] = 0; M[A] = 1; M[E] = z;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_scl_mat(float M[16], float x, float y, float z)
{
    M[0] = x; M[4] = 0; M[8] = 0; M[C] = 0;
    M[1] = 0; M[5] = y; M[9] = 0; M[D] = 0;
    M[2] = 0; M[6] = 0; M[A] = z; M[E] = 0;
    M[3] = 0; M[7] = 0; M[B] = 0; M[F] = 1;
}

void load_rot_mat(float M[16], float x, float y, float z, float a)
{
    float U[16], S[16], u[3], k = (float) sqrt(x * x + y * y + z * z);

    const float s = (float) sin((double) RAD(a));
    const float c = (float) cos((double) RAD(a));

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
// Matrix inverse constructors

void load_xlt_inv(float I[16], float x, float y, float z)
{
    load_xlt_mat(I, -x, -y, -z);
}

void load_scl_inv(float I[16], float x, float y, float z)
{
    load_scl_mat(I, 1 / x, 1 / y, 1 / z);
}

void load_rot_inv(float I[16], float x, float y, float z, float a)
{
    load_rot_mat(I, x, y, z, -a);
}

//-----------------------------------------------------------------------------
// Matrix left-composers

void Lmul_xlt_mat(float M[16], float x, float y, float z)
{
    float T[16];

    load_xlt_mat(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_scl_mat(float M[16], float x, float y, float z)
{
    float T[16];

    load_scl_mat(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_rot_mat(float M[16], float x, float y, float z, float a)
{
    float T[16];

    load_rot_mat(T, x, y, z, a);
    mult_mat_mat(M, T, M);
}

//-----------------------------------------------------------------------------
// Matrix inverse left-composers

void Lmul_xlt_inv(float M[16], float x, float y, float z)
{
    float T[16];

    load_xlt_inv(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_scl_inv(float M[16], float x, float y, float z)
{
    float T[16];

    load_scl_inv(T, x, y, z);
    mult_mat_mat(M, T, M);
}

void Lmul_rot_inv(float M[16], float x, float y, float z, float a)
{
    float T[16];

    load_rot_inv(T, x, y, z, a);
    mult_mat_mat(M, T, M);
}

//-----------------------------------------------------------------------------
// Matrix right-composers

void Rmul_xlt_mat(float M[16], float x, float y, float z)
{
    float T[16];

    load_xlt_mat(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_scl_mat(float M[16], float x, float y, float z)
{
    float T[16];

    load_scl_mat(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_rot_mat(float M[16], float x, float y, float z, float a)
{
    float T[16];

    load_rot_mat(T, x, y, z, a);
    mult_mat_mat(M, M, T);
}

//-----------------------------------------------------------------------------
// Matrix inverse right-composers

void Rmul_xlt_inv(float M[16], float x, float y, float z)
{
    float T[16];

    load_xlt_inv(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_scl_inv(float M[16], float x, float y, float z)
{
    float T[16];

    load_scl_inv(T, x, y, z);
    mult_mat_mat(M, M, T);
}

void Rmul_rot_inv(float M[16], float x, float y, float z, float a)
{
    float T[16];

    load_rot_inv(T, x, y, z, a);
    mult_mat_mat(M, M, T);
}

//-----------------------------------------------------------------------------
// Multipliers and transformers

void mult_mat_mat(float M[16], const float N[16], const float O[16])
{
    float T[16];

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

void mult_mat_pos(float v[3], const float M[16], const float u[3])
{
    v[0] = M[0] * u[0] + M[4] * u[1] + M[8] * u[2] + M[C];
    v[1] = M[1] * u[0] + M[5] * u[1] + M[9] * u[2] + M[D];
    v[2] = M[2] * u[0] + M[6] * u[1] + M[A] * u[2] + M[E];
}

void mult_mat_vec(float v[4], const float M[16], const float u[4])
{
    v[0] = M[0] * u[0] + M[4] * u[1] + M[8] * u[2] + M[C] * u[3];
    v[1] = M[1] * u[0] + M[5] * u[1] + M[9] * u[2] + M[D] * u[3];
    v[2] = M[2] * u[0] + M[6] * u[1] + M[A] * u[2] + M[E] * u[3];
    v[3] = M[3] * u[0] + M[7] * u[1] + M[B] * u[2] + M[F] * u[3];
}

void mult_xps_pos(float v[3], const float M[16], const float u[3])
{
    v[0] = M[0] * u[0] + M[1] * u[1] + M[2] * u[2] + M[3];
    v[1] = M[4] * u[0] + M[5] * u[1] + M[6] * u[2] + M[7];
    v[2] = M[8] * u[0] + M[9] * u[1] + M[A] * u[2] + M[B];
}

void mult_xps_vec(float v[4], const float M[16], const float u[4])
{
    v[0] = M[0] * u[0] + M[1] * u[1] + M[2] * u[2] + M[3] * u[3];
    v[1] = M[4] * u[0] + M[5] * u[1] + M[6] * u[2] + M[7] * u[3];
    v[2] = M[8] * u[0] + M[9] * u[1] + M[A] * u[2] + M[B] * u[3];
    v[3] = M[C] * u[0] + M[D] * u[1] + M[E] * u[2] + M[F] * u[3];
}

//-----------------------------------------------------------------------------
// Miscellaneous vector operations

void normalize(float v[3])
{
    float k = (float) sqrt(DOT3(v, v));

    v[0] /= k;
    v[1] /= k;
    v[2] /= k;
}

void cross(float u[3], const float v[3],
                       const float w[3])
{
    u[0] = v[1] * w[2] - v[2] * w[1];
    u[1] = v[2] * w[0] - v[0] * w[2];
    u[2] = v[0] * w[1] - v[1] * w[0];
}

//-----------------------------------------------------------------------------
// Quaternion / matrix conversions

void get_quaternion(float q[4], const float M[16])
{
    float t = 1 + M[0] + M[5] + M[10];

    if (t > 0.00001)
    {
        float s = float(0.5 / sqrt(t));

        q[0] = (M[6] - M[9]) * s;
        q[1] = (M[8] - M[2]) * s;
        q[2] = (M[1] - M[4]) * s;
        q[3] =         0.25f / s;
    }
    else if (M[0] > M[5] && M[0] > M[10])
    {
        float s = float(sqrt(1 + M[0] - M[5] - M[10]) * 2);

        q[0] =         0.25f * s;
        q[1] = (M[1] + M[4]) / s;
        q[2] = (M[8] + M[2]) / s;
        q[3] = (M[6] - M[9]) / s;
    }
    else if (M[5] > M[10])
    {
        float s = float(sqrt(1 + M[5] - M[0] - M[10]) * 2);

        q[0] = (M[1] + M[4]) / s;
        q[1] =         0.25f * s;
        q[2] = (M[6] + M[9]) / s;
        q[3] = (M[8] - M[2]) / s;
    }
    else
    {
        float s = float(sqrt(1 + M[10] - M[0] - M[5]) * 2);

        q[0] = (M[8] + M[2]) / s;
        q[1] = (M[6] + M[9]) / s;
        q[2] =         0.25f * s;
        q[3] = (M[1] - M[4]) / s;
    }
}

void set_quaternion(float M[16], const float q[4])
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

//-----------------------------------------------------------------------------
