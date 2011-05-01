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

#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <cmath>
#include <limits>

/*---------------------------------------------------------------------------*/

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef PI_2
#define PI_2 1.57079632679489661923
#endif

#ifndef DEG
#define DEG(r) (180.0 * (r) / PI)
#endif

#ifndef RAD
#define RAD(d) (PI * (d) / 180.0)
#endif

/*---------------------------------------------------------------------------*/

#define DOT3(v, w) ((v)[0] * (w)[0] + \
                    (v)[1] * (w)[1] + \
                    (v)[2] * (w)[2])
#define DOT4(v, w) ((v)[0] * (w)[0] + \
                    (v)[1] * (w)[1] + \
                    (v)[2] * (w)[2] + \
                    (v)[3] * (w)[3])

//-----------------------------------------------------------------------------

void load_idt(double *);

void load_mat(double *, const double *);
void load_xps(double *, const double *);
void load_inv(double *, const double *);

//-----------------------------------------------------------------------------

void load_xlt_mat(double *, double, double, double);
void load_scl_mat(double *, double, double, double);
void load_rot_mat(double *, double, double, double, double);

void load_xlt_inv(double *, double, double, double);
void load_scl_inv(double *, double, double, double);
void load_rot_inv(double *, double, double, double, double);

void load_persp(double *, double, double, double, double, double, double);
void load_ortho(double *, double, double, double, double, double, double);

//-----------------------------------------------------------------------------

void Lmul_xlt_mat(double *, double, double, double);
void Lmul_scl_mat(double *, double, double, double);
void Lmul_rot_mat(double *, double, double, double, double);

void Lmul_xlt_inv(double *, double, double, double);
void Lmul_scl_inv(double *, double, double, double);
void Lmul_rot_inv(double *, double, double, double, double);

//-----------------------------------------------------------------------------

void Rmul_xlt_mat(double *, double, double, double);
void Rmul_scl_mat(double *, double, double, double);
void Rmul_rot_mat(double *, double, double, double, double);

void Rmul_xlt_inv(double *, double, double, double);
void Rmul_scl_inv(double *, double, double, double);
void Rmul_rot_inv(double *, double, double, double, double);

//-----------------------------------------------------------------------------

void mult_mat_mat(double *, const double *, const double *);

void mult_mat_vec3(double *, const double *, const double *);
void mult_xps_vec3(double *, const double *, const double *);
void mult_mat_vec4(double *, const double *, const double *);
void mult_xps_vec4(double *, const double *, const double *);

//-----------------------------------------------------------------------------

#define v_cp2(u, v) { \
    (u)[0] = (v)[0];  \
    (u)[1] = (v)[1];  \
}

#define v_cp3(u, v) { \
    (u)[0] = (v)[0];  \
    (u)[1] = (v)[1];  \
    (u)[2] = (v)[2];  \
}

//-----------------------------------------------------------------------------

#define normalize(v) {                          \
    double k = 1.0 / sqrt(DOT3((v), (v)));      \
    (v)[0] *= k;                                \
    (v)[1] *= k;                                \
    (v)[2] *= k;                                \
}

#define bisection(u, v, w) {                    \
    (u)[0] = (v)[0] + (w)[0];                   \
    (u)[1] = (v)[1] + (w)[1];                   \
    (u)[2] = (v)[2] + (w)[2];                   \
    normalize(u);                               \
}

#define crossprod(u, v, w) {                    \
    (u)[0] = (v)[1] * (w)[2] - (v)[2] * (w)[1]; \
    (u)[1] = (v)[2] * (w)[0] - (v)[0] * (w)[2]; \
    (u)[2] = (v)[0] * (w)[1] - (v)[1] * (w)[0]; \
}

void   midpoint(double *, const double *, const double *);
double distance(          const double *, const double *);

//-----------------------------------------------------------------------------

void mat_to_quat(double *, const double *);
void quat_to_mat(double *, const double *);

void orthonormalize(double *);

//-----------------------------------------------------------------------------

void set_basis(double *, const double *, const double *, const double *);
void set_plane(double *, const double *, const double *, const double *);

double solid_angle(const double *, const double *, const double *);

//-----------------------------------------------------------------------------

void sphere_to_vector(double *, double, double, double);
void vector_to_sphere(double *, double, double, double);

void slerp(double *, const double *, const double *, double);

//-----------------------------------------------------------------------------

int nearest_int(double);
int next_pow2(int);

//-----------------------------------------------------------------------------

#endif
