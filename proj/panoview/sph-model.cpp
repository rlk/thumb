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

#include <GL/glew.h>
#include <cstdio>
#include <cmath>

#include "math3d.h"
#include "cube.hpp"

#include "sph-model.hpp"

//------------------------------------------------------------------------------

sph_model::sph_model()
{
}

sph_model::~sph_model()
{
}

//------------------------------------------------------------------------------

struct face
{
    // Invariant: Corner vectors are normalized.
    
    double a[3];
    double b[3];
    double c[3];
    double d[3];
    
    void divide(face&, face&, face&, face&);
    
    bool test(const double *);
    void draw(const double *, int, int, int);
};

void face::divide(face& A, face& B, face &C, face &D)
{
    double n[3];
    double s[3];
    double e[3];
    double w[3];
    double m[3];

    // Find the midpoints of the sides of this face.

    vadd(n, a, b);
    vadd(s, c, d);
    vadd(e, a, c);
    vadd(w, b, d);
    vadd(m, n, s);

    // Normalize these giving points on the unit sphere.

    vnormalize(n, n);
    vnormalize(s, s);
    vnormalize(e, e);
    vnormalize(w, w);
    vnormalize(m, m);

    // Assign the corners of the four sub-faces.
    
    vcpy(A.a, a);
    vcpy(A.b, n);
    vcpy(A.c, e);
    vcpy(A.d, m);
    
    vcpy(B.a, n);
    vcpy(B.b, b);
    vcpy(B.c, m);
    vcpy(B.d, w);
    
    vcpy(C.a, e);
    vcpy(C.b, m);
    vcpy(C.c, c);
    vcpy(C.d, s);
    
    vcpy(D.a, m);
    vcpy(D.b, w);
    vcpy(D.c, s);
    vcpy(D.d, d);
}

bool face::test(const double *T)
{
    double m[3];
    
    m[0] = a[0] + b[0] + c[0] + d[0];
    m[1] = a[1] + b[1] + c[1] + d[1];
    m[2] = a[2] + b[2] + c[2] + d[2];
    
    double r1 = vlen(m) / vdot(a, m);
    double r0 = 1;
    
    const double da = vdot(a, T + 12);
    const double db = vdot(b, T + 12);
    const double dc = vdot(c, T + 12);
    const double dd = vdot(d, T + 12);
    
    double na, a0, a1;
    double nb, b0, b1;
    double nc, c0, c1;
    double nd, d0, d1;

    // X axis

    na = vdot(a, T + 0);
    nb = vdot(b, T + 0);
    nc = vdot(c, T + 0);
    nd = vdot(d, T + 0);

    a0 = (na * r0 + T[3]) / (da * r0 + T[15]);
    a1 = (na * r1 + T[3]) / (da * r1 + T[15]);
    b0 = (nb * r0 + T[3]) / (db * r0 + T[15]);
    b1 = (nb * r1 + T[3]) / (db * r1 + T[15]);
    c0 = (nc * r0 + T[3]) / (dc * r0 + T[15]);
    c1 = (nc * r1 + T[3]) / (dc * r1 + T[15]);
    d0 = (nd * r0 + T[3]) / (dd * r0 + T[15]);
    d1 = (nd * r1 + T[3]) / (dd * r1 + T[15]);
    
    if (a0 < -1 && b0 < -1 && c0 < -1 && d0 < -1 &&
        a1 < -1 && b1 < -1 && c1 < -1 && d1 < -1) return false;
    if (a0 >  1 && b0 >  1 && c0 >  1 && d0 >  1 &&
        a1 >  1 && b1 >  1 && c1 >  1 && d1 >  1) return false;

    // Y axis

    na = vdot(a, T + 4);
    nb = vdot(b, T + 4);
    nc = vdot(c, T + 4);
    nd = vdot(d, T + 4);

    a0 = (na * r0 + T[7]) / (da * r0 + T[15]);
    a1 = (na * r1 + T[7]) / (da * r1 + T[15]);
    b0 = (nb * r0 + T[7]) / (db * r0 + T[15]);
    b1 = (nb * r1 + T[7]) / (db * r1 + T[15]);
    c0 = (nc * r0 + T[7]) / (dc * r0 + T[15]);
    c1 = (nc * r1 + T[7]) / (dc * r1 + T[15]);
    d0 = (nd * r0 + T[7]) / (dd * r0 + T[15]);
    d1 = (nd * r1 + T[7]) / (dd * r1 + T[15]);
    
    if (a0 < -1 && b0 < -1 && c0 < -1 && d0 < -1 &&
        a1 < -1 && b1 < -1 && c1 < -1 && d1 < -1) return false;
    if (a0 >  1 && b0 >  1 && c0 >  1 && d0 >  1 &&
        a1 >  1 && b1 >  1 && c1 >  1 && d1 >  1) return false;

    return true;
}

void face::draw(const double *T, int w, int h, int l)
{
    if (test(T))
    {
        if (l == 0)
        {
            glBegin(GL_QUADS);
            {
                glNormal3dv(a);
                glVertex3dv(a);           
                glNormal3dv(b);
                glVertex3dv(b);
                glNormal3dv(d);
                glVertex3dv(d);
                glNormal3dv(c);
                glVertex3dv(c);
            }
            glEnd();
        }
        else
        {
            face A;
            face B;
            face C;
            face D;
            
            divide(A, B, C, D);
            
            A.draw(T, w, h, l - 1);
            B.draw(T, w, h, l - 1);
            C.draw(T, w, h, l - 1);
            D.draw(T, w, h, l - 1);
        }
    }
}

//------------------------------------------------------------------------------

void sph_model::draw(const double *P, const double *V, int w, int h)
{
    double M[16];
    double T[16];
    
    mmultiply (M, P, V);
    mtranspose(T, M);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(0);

    glColor4d(1.0, 1.0, 1.0, 0.2);

    for (int i = 0; i < 6; ++i)
    {
        face F;
        
        vnormalize(F.a, cube_v[cube_i[i][0]]);
        vnormalize(F.b, cube_v[cube_i[i][1]]);
        vnormalize(F.c, cube_v[cube_i[i][2]]);
        vnormalize(F.d, cube_v[cube_i[i][3]]);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        F.draw(T, w, h, 4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        F.draw(T, w, h, 4);
    }
}

//------------------------------------------------------------------------------
