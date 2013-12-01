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

#ifndef ETC_VECTOR_HPP
#define ETC_VECTOR_HPP

#include <cmath>
#include <limits>

//------------------------------------------------------------------------------

#ifndef PI
#define PI 3.141592653589793
#endif

#ifndef SQRT2
#define SQRT2 1.414213562373095
#endif

//------------------------------------------------------------------------------

struct vec3;
struct vec4;
struct quat;
struct mat3;
struct mat4;

//------------------------------------------------------------------------------

/// 3-component double precision floating point vector.

struct vec3
{
    double v[3];

    vec3(double x=0, double y=0, double z=0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    vec3(const vec4&);

    operator const double*() const
    {
        return const_cast<double *>(&v[0]);
    }

    const double& operator[](int i) const { return v[i]; }
          double& operator[](int i)       { return v[i]; }
};

//------------------------------------------------------------------------------

/// 4-component double precision floating point vector.

struct vec4
{
    double v[4];

    vec4(double x=0, double y=0, double z=0, double w=0)
    {
        v[0] = x;
        v[1] = y;
        v[2] = z;
        v[3] = w;
    }
    vec4(const vec3& a, double b)
    {
        v[0] = a[0];
        v[1] = a[1];
        v[2] = a[2];
        v[3] = b;
    }

    operator const double*() const
    {
        return const_cast<double *>(&v[0]);
    }

    const double& operator[](int i) const { return v[i]; }
          double& operator[](int i)       { return v[i]; }
};

//------------------------------------------------------------------------------

/// Double precision floating point quaternion.

struct quat
{
    double q[4];

    quat(double x=0, double y=0, double z=0, double w=1)
    {
        q[0] = x;
        q[1] = y;
        q[2] = z;
        q[3] = w;
    }
    quat(const mat3&);
    quat(const vec3&, double);

    operator const double*() const
    {
        return const_cast<double *>(&q[0]);
    }

    const double& operator[](int i) const { return q[i]; }
          double& operator[](int i)       { return q[i]; }
};

//------------------------------------------------------------------------------

/// Row-wise 3x3 double precision floating point matrix.

struct mat3
{
    vec3 M[3];

    mat3(double m00=1, double m01=0, double m02=0,
         double m10=0, double m11=1, double m12=0,
         double m20=0, double m21=0, double m22=1)
    {
        M[0] = vec3(m00, m01, m02);
        M[1] = vec3(m10, m11, m12);
        M[2] = vec3(m20, m21, m22);
    }
    mat3(const quat&);
    mat3(const vec3&, const vec3&, const vec3&);

    const vec3& operator[](int i) const { return M[i]; }
          vec3& operator[](int i)       { return M[i]; }

    operator const double*() const
    {
        return const_cast<double *>(&M[0][0]);
    }
};

//------------------------------------------------------------------------------

/// Row-wise 4x4 double precision floating point matrix.

struct mat4
{
    vec4 M[4];

    mat4(double m00=1, double m01=0, double m02=0, double m03=0,
         double m10=0, double m11=1, double m12=0, double m13=0,
         double m20=0, double m21=0, double m22=1, double m23=0,
         double m30=0, double m31=0, double m32=0, double m33=1)
    {
        M[0] = vec4(m00, m01, m02, m03);
        M[1] = vec4(m10, m11, m12, m13);
        M[2] = vec4(m20, m21, m22, m23);
        M[3] = vec4(m30, m31, m32, m33);
    }
    mat4(const mat3& A)
    {
        M[0] = vec4(A[0],    0);
        M[1] = vec4(A[1],    0);
        M[2] = vec4(A[2],    0);
        M[3] = vec4(0, 0, 0, 1);
    }

    const vec4& operator[](int i) const { return M[i]; }
          vec4& operator[](int i)       { return M[i]; }

    operator const double*() const
    {
        return const_cast<double *>(&M[0][0]);
    }
};

//------------------------------------------------------------------------------

/// Convert an angle in degrees to an angle in radians.

inline double to_radians(double degrees)
{
    return degrees * 0.01745329251994330;
}

/// Convent an angle in radians to an angle in degrees.

inline double to_degrees(double radians)
{
    return radians * 57.29577951308232;
}

//------------------------------------------------------------------------------

/// Calculate the 3-component negation of v.

inline vec3 operator-(const vec3& v)
{
    return vec3(-v[0], -v[1], -v[2]);
}

/// Calculate the 3-component sum of v and w.

inline vec3 operator+(const vec3& v, const vec3& w)
{
    return vec3(v[0] + w[0], v[1] + w[1], v[2] + w[2]);
}

/// Calculate the 3-component difference of v and w.

inline vec3 operator-(const vec3& v, const vec3& w)
{
    return vec3(v[0] - w[0], v[1] - w[1], v[2] - w[2]);
}

/// Calculate the 3-component scalar quotient of v and k.

inline vec3 operator/(const vec3& v, double k)
{
    return vec3(v[0] / k, v[1] / k, v[2] / k);
}

/// Calculate the 3-component scalar product of v and k.

inline vec3 operator*(const vec3& v, double k)
{
    return vec3(v[0] * k, v[1] * k, v[2] * k);
}

/// Compute the cross product of v and w.

inline vec3 cross(const vec3& v, const vec3& w)
{
    return vec3(v[1] * w[2] - v[2] * w[1],
                v[2] * w[0] - v[0] * w[2],
                v[0] * w[1] - v[1] * w[0]);
}

//------------------------------------------------------------------------------

/// Calculate the quaternion sum of q and p.

inline quat operator+(const quat& q, const quat& p)
{
    return quat(q[0] + p[0], q[1] + p[1], q[2] + p[2], q[3] + p[3]);
}

/// Calculate the quaternion difference of q and p.

inline quat operator-(const quat& q, const quat& p)
{
    return quat(q[0] - p[0], q[1] - p[1], q[2] - p[2], q[3] - p[3]);
}

/// Calculate the quaternion scalar quotient of q and k.

inline quat operator/(const quat& q, double k)
{
    return quat(q[0] / k, q[1] / k, q[2] / k, q[3] / k);
}

/// Calculate the quaternion scalar product of q and k.

inline quat operator*(const quat& q, double k)
{
    return quat(q[0] * k, q[1] * k, q[2] * k, q[3] * k);
}

/// Calculate the quaternion product of q and p.

inline quat operator*(const quat& q, const quat& p)
{
    return quat(q[0] * p[3] + q[3] * p[0] + q[1] * p[2] - q[2] * p[1],
                q[1] * p[3] + q[3] * p[1] + q[2] * p[0] - q[0] * p[2],
                q[2] * p[3] + q[3] * p[2] + q[0] * p[1] - q[1] * p[0],
                q[3] * p[3] - q[0] * p[0] - q[1] * p[1] - q[2] * p[2]);
}

//------------------------------------------------------------------------------

/// Calculate the 3-component dot product of v and w.

inline double operator*(const vec3& v, const vec3& w)
{
    return v[0] * w[0] + v[1] * w[1] + v[2] * w[2];
}

/// Calculate the 4-component dot product of v and w.

inline double operator*(const vec4& v, const vec4& w)
{
    return v[0] * w[0] + v[1] * w[1] + v[2] * w[2] + v[3] * w[3];
}

//------------------------------------------------------------------------------

/// Calculate the 3-component transform of vector v by 3-matrix A.

inline vec3 operator*(const mat3& A, const vec3& v)
{
    return vec3(A[0] * v, A[1] * v, A[2] * v);
}

/// Calculate the 3-component transform of vector v by 4-matrix A.

inline vec3 operator*(const mat4& A, const vec3& v)
{
    return vec3(A[0] * vec4(v, 1), A[1] * vec4(v, 1), A[2] * vec4(v, 1));
}

/// Calculate the 4-component transform of vector v by 4-matrix A.

inline vec4 operator*(const mat4& A, const vec4& v)
{
    return vec4(A[0] * v, A[1] * v, A[2] * v, A[3] * v);
}

//------------------------------------------------------------------------------

/// Calculate the 3x3 matrix product of A and B.

inline mat3 operator*(const mat3& A, const mat3& B)
{
    mat3 M;
    for     (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            M[i][j] = A[i][0] * B[0][j]
                    + A[i][1] * B[1][j]
                    + A[i][2] * B[2][j];
    return M;
}

/// Calculate the 4x4 matrix product of A and B.

inline mat4 operator*(const mat4& A, const mat4& B)
{
    mat4 M;
    for     (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = A[i][0] * B[0][j]
                    + A[i][1] * B[1][j]
                    + A[i][2] * B[2][j]
                    + A[i][3] * B[3][j];
    return M;
}

//------------------------------------------------------------------------------

/// Return the transpose of a 3x3 matrix.

inline mat3 transpose(const mat3& A)
{
    mat3 M;
    for     (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            M[i][j] = A[j][i];
    return M;
}

/// Return the transpose of a 4x4 matrix.

inline mat4 transpose(const mat4& A)
{
    mat4 M;
    for     (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            M[i][j] = A[j][i];
    return M;
}

//------------------------------------------------------------------------------

/// Return the conjugate of quaternion q.

inline quat conjugate(const quat& q)
{
    return quat(-q[0], -q[1], -q[2], +q[3]);
}

/// Return the inverse of quaternion q.

inline quat inverse(const quat& q)
{
    return conjugate(q) / (q[0] * q[0] +
                           q[1] * q[1] +
                           q[2] * q[2] +
                           q[3] * q[3]);
}

/// Return the inverse of 4x4 matrix A.

inline mat4 inverse(const mat4& A)
{
    mat4 T;

    T[0][0] = +(A[1][1] * (A[2][2] * A[3][3] - A[3][2] * A[2][3]) -
                A[1][2] * (A[2][1] * A[3][3] - A[3][1] * A[2][3]) +
                A[1][3] * (A[2][1] * A[3][2] - A[3][1] * A[2][2]));
    T[0][1] = -(A[1][0] * (A[2][2] * A[3][3] - A[3][2] * A[2][3]) -
                A[1][2] * (A[2][0] * A[3][3] - A[3][0] * A[2][3]) +
                A[1][3] * (A[2][0] * A[3][2] - A[3][0] * A[2][2]));
    T[0][2] = +(A[1][0] * (A[2][1] * A[3][3] - A[3][1] * A[2][3]) -
                A[1][1] * (A[2][0] * A[3][3] - A[3][0] * A[2][3]) +
                A[1][3] * (A[2][0] * A[3][1] - A[3][0] * A[2][1]));
    T[0][3] = -(A[1][0] * (A[2][1] * A[3][2] - A[3][1] * A[2][2]) -
                A[1][1] * (A[2][0] * A[3][2] - A[3][0] * A[2][2]) +
                A[1][2] * (A[2][0] * A[3][1] - A[3][0] * A[2][1]));

    T[1][0] = -(A[0][1] * (A[2][2] * A[3][3] - A[3][2] * A[2][3]) -
                A[0][2] * (A[2][1] * A[3][3] - A[3][1] * A[2][3]) +
                A[0][3] * (A[2][1] * A[3][2] - A[3][1] * A[2][2]));
    T[1][1] = +(A[0][0] * (A[2][2] * A[3][3] - A[3][2] * A[2][3]) -
                A[0][2] * (A[2][0] * A[3][3] - A[3][0] * A[2][3]) +
                A[0][3] * (A[2][0] * A[3][2] - A[3][0] * A[2][2]));
    T[1][2] = -(A[0][0] * (A[2][1] * A[3][3] - A[3][1] * A[2][3]) -
                A[0][1] * (A[2][0] * A[3][3] - A[3][0] * A[2][3]) +
                A[0][3] * (A[2][0] * A[3][1] - A[3][0] * A[2][1]));
    T[1][3] = +(A[0][0] * (A[2][1] * A[3][2] - A[3][1] * A[2][2]) -
                A[0][1] * (A[2][0] * A[3][2] - A[3][0] * A[2][2]) +
                A[0][2] * (A[2][0] * A[3][1] - A[3][0] * A[2][1]));

    T[2][0] = +(A[0][1] * (A[1][2] * A[3][3] - A[3][2] * A[1][3]) -
                A[0][2] * (A[1][1] * A[3][3] - A[3][1] * A[1][3]) +
                A[0][3] * (A[1][1] * A[3][2] - A[3][1] * A[1][2]));
    T[2][1] = -(A[0][0] * (A[1][2] * A[3][3] - A[3][2] * A[1][3]) -
                A[0][2] * (A[1][0] * A[3][3] - A[3][0] * A[1][3]) +
                A[0][3] * (A[1][0] * A[3][2] - A[3][0] * A[1][2]));
    T[2][2] = +(A[0][0] * (A[1][1] * A[3][3] - A[3][1] * A[1][3]) -
                A[0][1] * (A[1][0] * A[3][3] - A[3][0] * A[1][3]) +
                A[0][3] * (A[1][0] * A[3][1] - A[3][0] * A[1][1]));
    T[2][3] = -(A[0][0] * (A[1][1] * A[3][2] - A[3][1] * A[1][2]) -
                A[0][1] * (A[1][0] * A[3][2] - A[3][0] * A[1][2]) +
                A[0][2] * (A[1][0] * A[3][1] - A[3][0] * A[1][1]));

    T[3][0] = -(A[0][1] * (A[1][2] * A[2][3] - A[2][2] * A[1][3]) -
                A[0][2] * (A[1][1] * A[2][3] - A[2][1] * A[1][3]) +
                A[0][3] * (A[1][1] * A[2][2] - A[2][1] * A[1][2]));
    T[3][1] = +(A[0][0] * (A[1][2] * A[2][3] - A[2][2] * A[1][3]) -
                A[0][2] * (A[1][0] * A[2][3] - A[2][0] * A[1][3]) +
                A[0][3] * (A[1][0] * A[2][2] - A[2][0] * A[1][2]));
    T[3][2] = -(A[0][0] * (A[1][1] * A[2][3] - A[2][1] * A[1][3]) -
                A[0][1] * (A[1][0] * A[2][3] - A[2][0] * A[1][3]) +
                A[0][3] * (A[1][0] * A[2][1] - A[2][0] * A[1][1]));
    T[3][3] = +(A[0][0] * (A[1][1] * A[2][2] - A[2][1] * A[1][2]) -
                A[0][1] * (A[1][0] * A[2][2] - A[2][0] * A[1][2]) +
                A[0][2] * (A[1][0] * A[2][1] - A[2][0] * A[1][1]));

    const double d = 1.0 / (A[0] * T[0]);

    return mat4(T[0][0] * d, T[1][0] * d, T[2][0] * d, T[3][0] * d,
                T[0][1] * d, T[1][1] * d, T[2][1] * d, T[3][1] * d,
                T[0][2] * d, T[1][2] * d, T[2][2] * d, T[3][2] * d,
                T[0][3] * d, T[1][3] * d, T[2][3] * d, T[3][3] * d);
}

//------------------------------------------------------------------------------

/// Compute the length of vector v.

inline double length(const vec3 &v)
{
    return sqrt(v * v);
}

/// Compute the normalization of vector v.

inline vec3 normal(const vec3& v)
{
    return v / length(v);
}

/// Compute the normalization of quaternion q;

inline quat normal(const quat& q)
{
    return q / sqrt(q[0] * q[0] +
                    q[1] * q[1] +
                    q[2] * q[2] +
                    q[3] * q[3]);
}

//------------------------------------------------------------------------------

/// Return the X vector of the coordinate system given by matrix M.

inline vec3 xvector(const mat4& M)
{
    return vec3(M[0][0], M[1][0], M[2][0]);
}

/// Return the Y vector of the coordinate system given by matrix M.

inline vec3 yvector(const mat4& M)
{
    return vec3(M[0][1], M[1][1], M[2][1]);
}

/// Return the Z vector of the coordinate system given by matrix M.

inline vec3 zvector(const mat4& M)
{
    return vec3(M[0][2], M[1][2], M[2][2]);
}

/// Return the W vector (position) of the coordinate system given by matrix M.

inline vec3 wvector(const mat4& M)
{
    return vec3(M[0][3], M[1][3], M[2][3]);
}

//------------------------------------------------------------------------------

/// Return a matrix giving a rotation about X through a radians.

inline mat4 xrotation(double a)
{
    return mat4(1, 0,       0,      0,
                0, cos(a), -sin(a), 0,
                0, sin(a),  cos(a), 0,
                0, 0,       0,      1);
}

/// Return a matrix giving a rotation about Y through a radians.

inline mat4 yrotation(double a)
{
    return mat4( cos(a), 0, sin(a), 0,
                 0,      1, 0,      0,
                -sin(a), 0, cos(a), 0,
                 0,      0, 0,      1);
}

/// Return a matrix giving a rotation about Z through a radians.

inline mat4 zrotation(double a)
{
    return mat4(cos(a), -sin(a), 0, 0,
                sin(a),  cos(a), 0, 0,
                0,       0,      1, 0,
                0,       0,      0, 1);
}

/// Return a matrix giving a rotation about v through a radians.

inline mat4 rotation(const vec3& v, double a)
{
    const vec3   u = normal(v);
    const double s = sin(a);
    const double c = cos(a);

    return mat4(u[0] * u[0] + (1 - u[0] * u[0]) * c,
                u[0] * u[1] + (0 - u[0] * u[1]) * c - u[2] * s,
                u[0] * u[2] + (0 - u[0] * u[2]) * c + u[1] * s,
                0,
                u[1] * u[0] + (0 - u[1] * u[0]) * c + u[2] * s,
                u[1] * u[1] + (1 - u[1] * u[1]) * c,
                u[1] * u[2] + (0 - u[1] * u[2]) * c - u[0] * s,
                0,
                u[2] * u[0] + (0 - u[2] * u[0]) * c - u[1] * s,
                u[2] * u[1] + (0 - u[2] * u[1]) * c + u[0] * s,
                u[2] * u[2] + (1 - u[2] * u[2]) * c,
                0,
                0, 0, 0, 1);
}

/// Return a matrix giving a translation along vector v.

inline mat4 translation(const vec3& v)
{
    return mat4(1, 0, 0, v[0],
                0, 1, 0, v[1],
                0, 0, 1, v[2],
                0, 0, 0, 1);
}

/// Return a matrix giving a scale along vector v.

inline mat4 scale(const vec3& v)
{
    return mat4(v[0], 0,    0,    0,
                0,    v[1], 0,    0,
                0,    0,    v[2], 0,
                0,    0,    0,    1);
}

/// Return a matrix giving a perspective projection with field-of-view v,
/// aspect ratio a, near clipping distance n, and far clipping distance f.

inline mat4 perspective(double v, double a, double n, double f)
{
    const double y = n * tan(v / 2);
    const double x = y * a;

    return mat4(n / x, 0, 0, 0, 0,
                n / y, 0, 0, 0, 0, (n + f) / (n - f),
                               2 * (n * f) / (n - f), 0, 0, -1, 0);
}

/// Return a matrix giving a perspective projection with the given left,
/// right, bottom, top, near, and far clipping boundaries.

inline mat4 perspective(double l, double r,
                        double b, double t,
                        double n, double f)
{
    return mat4((n + n) / (r - l), 0,
                (r + l) / (r - l), 0, 0,
                (n + n) / (t - b),
                (t + b) / (t - b), 0, 0, 0,
                (n + f) / (n - f),
            2 * (n * f) / (n - f), 0, 0, -1, 0);
}

/// Return a matrix giving an orthogonal projection with the given left,
/// right, bottom, top, near, and far clipping boundaries.

inline mat4 orthogonal(double l, double r,
                       double b, double t,
                       double n, double f)
{
     return mat4(2 / (r - l), 0, 0, -(r + l) / (r - l), 0,
                 2 / (t - b), 0,    -(t + b) / (t - b), 0, 0,
                -2 / (f - n),       -(f + n) / (f - n), 0, 0, 0, 1);
}

//------------------------------------------------------------------------------

inline vec3 mix(const vec3& u, const vec3& v, double t)
{
    return u * (1.0 - t) + v * t;
}

/// Return the spherical linear interpolation of quaternions q and p at t.

inline quat slerp(const quat& q, const quat& p, double t)
{
    const double d = q[0] * p[0] + q[1] * p[1] + q[2] * p[2] + q[3] * p[3];
    const double k = acos(fabs(d));

    const double u = sin(k - t * k) / sin(k);
    const double v = sin(    t * k) / sin(k);

    if (fabs(d) < 1.0)
    {
        if (d > 0.0)
            return q * u + p * v;
        else
            return q * u - p * v;
    }
    else return q;
}

//------------------------------------------------------------------------------

/// Drop the fourth component of a vec4. (This conforms to GLSL.)

inline vec3::vec3(const vec4& w)
{
    v[0] = w[0];
    v[1] = w[1];
    v[2] = w[2];
}

/// Construct a quaternion from rotation matrix M.

inline quat::quat(const mat3& M)
{
    if (1.0 + M[0][0] + M[1][1] + M[2][2] > 0.0)
    {
        const double s = 0.5 / sqrt(1.0 + M[0][0] + M[1][1] + M[2][2]);
        q[1] = (M[0][2] - M[2][0]) * s;
        q[2] = (M[1][0] - M[0][1]) * s;
        q[0] = (M[2][1] - M[1][2]) * s;
        q[3] =               0.25  / s;
    }
    else if (M[0][0] > M[1][1] && M[0][0] > M[2][2])
    {
        const double s = 2.0 * sqrt(1.0 + M[0][0] - M[1][1] - M[2][2]);
        q[2] = (M[0][2] + M[2][0]) / s;
        q[1] = (M[1][0] + M[0][1]) / s;
        q[3] = (M[2][1] - M[1][2]) / s;
        q[0] =               0.25  * s;
    }
    else if (M[1][1] > M[2][2])
    {
        const double s = 2.0 * sqrt(1.0 + M[1][1] - M[0][0] - M[2][2]);
        q[3] = (M[0][2] - M[2][0]) / s;
        q[0] = (M[1][0] + M[0][1]) / s;
        q[2] = (M[2][1] + M[1][2]) / s;
        q[1] =               0.25  * s;
    }
    else
    {
        const double s = 2.0 * sqrt(1.0 + M[2][2] - M[0][0] - M[1][1]);
        q[0] = (M[0][2] + M[2][0]) / s;
        q[3] = (M[1][0] - M[0][1]) / s;
        q[1] = (M[2][1] + M[1][2]) / s;
        q[2] =               0.25  * s;
    }
}

/// Construct a rotation matrix from quaternion q.

inline mat3::mat3(const quat& q)
{
    M[0] = vec3(1 - 2 * (q[1] * q[1] + q[2] * q[2]),
                    2 * (q[0] * q[1] - q[2] * q[3]),
                    2 * (q[0] * q[2] + q[1] * q[3]));
    M[1] = vec3(    2 * (q[0] * q[1] + q[2] * q[3]),
                1 - 2 * (q[0] * q[0] + q[2] * q[2]),
                    2 * (q[1] * q[2] - q[0] * q[3]));
    M[2] = vec3(    2 * (q[0] * q[2] - q[1] * q[3]),
                    2 * (q[1] * q[2] + q[0] * q[3]),
                1 - 2 * (q[0] * q[0] + q[1] * q[1]));
}

/// Construct a quaternion from axis v and angle a in radians.

inline quat::quat(const vec3& v, double a)
{
    const double s = sin(a / 2);
    const double c = cos(a / 2);

    q[0] = v[0] * s;
    q[1] = v[1] * s;
    q[2] = v[2] * s;
    q[3] =        c;
}

/// Construct a rotation matrix from a set of basis vectors.

inline mat3::mat3(const vec3& x, const vec3& y, const vec3& z)
{
    M[0] = vec3(x[0], y[0], z[0]);
    M[1] = vec3(x[1], y[1], z[1]);
    M[2] = vec3(x[2], y[2], z[2]);
}

//-----------------------------------------------------------------------------

/// Round to the nearest integer. Round 0.5 toward negative infinity.

inline int toint(double d)
{
    double f = floor(d);
    double c = ceil (d);

    return int((c - d < d - f) ? c : f);
}

/// Return a power of two greater than or equal to n.

inline unsigned int topow2(unsigned int n)
{
    n--;
    n |= n >>  1;
    n |= n >>  2;
    n |= n >>  4;
    n |= n >>  8;
    n |= n >> 16;
    n++;

    return n;
}

//------------------------------------------------------------------------------

#endif
