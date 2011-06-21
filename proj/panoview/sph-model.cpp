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
#include "glyph.h"
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

static inline bool project(double *a, const double *M, const double *b)
{
    double d = (M[ 3] * b[0] + M[ 7] * b[1] + M[11] * b[2] + M[15]);
    
    if (fabs(d) > 0)
    {
        a[2] = (M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14]) / d;
        a[1] = (M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13]) / d;
        a[0] = (M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12]) / d;
        return true;
    }
    else
        return false;
}

static inline double length(const double *a, const double *b, int w, int h)
{
    if (finite(a[0]) && finite(a[1]) && finite(b[0]) && finite(b[1]))
    {
        double dx = (a[0] - b[0]) * w / 2;
        double dy = (a[1] - b[1]) * h / 2;
        
        return sqrt(dx * dx + dy * dy);
    }
    else return 0;
}

static inline double max(double a, double b, double c, double d)
{
    double x = (a > b) ? a : b;
    double y = (c > d) ? c : d;
    return     (x > y) ? x : y;
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
    
    double evaluate(const double *, int, int);
    
    void label(const char *, int);
    
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

double face::evaluate(const double *M, int w, int h)
{
    // Compute the maximum extent due to bulge.

    double t[3];
    
    t[0] = a[0] + b[0] + c[0] + d[0];
    t[1] = a[1] + b[1] + c[1] + d[1];
    t[2] = a[2] + b[2] + c[2] + d[2];
    
    double r = vlen(t) / vdot(a, t);

    // Compute the outer corners of the bulge bound.
    
    double A[3];
    double B[3];
    double C[3];
    double D[3];

    vmul(A, a, r);
    vmul(B, b, r);
    vmul(C, c, r);
    vmul(D, d, r);
    
    // Compute normalized device coordinates for all eight corners.
    
    double na[3], nA[3];
    double nb[3], nB[3];
    double nc[3], nC[3];
    double nd[3], nD[3];
    
    if (!project(na, M, a)) return 0;
    if (!project(nb, M, b)) return 0;
    if (!project(nc, M, c)) return 0;
    if (!project(nd, M, d)) return 0;
    if (!project(nA, M, A)) return 0;
    if (!project(nB, M, B)) return 0;
    if (!project(nC, M, C)) return 0;
    if (!project(nD, M, D)) return 0;

    // Check that any part of the boundary is visible.

    static const double k = 1;

    if (na[0] >  k && nb[0] >  k && nc[0] >  k && nd[0] >  k &&
        nA[0] >  k && nB[0] >  k && nC[0] >  k && nD[0] >  k) return 0;
    if (na[0] < -k && nb[0] < -k && nc[0] < -k && nd[0] < -k &&
        nA[0] < -k && nB[0] < -k && nC[0] < -k && nD[0] < -k) return 0;
    if (na[1] >  k && nb[1] >  k && nc[1] >  k && nd[1] >  k &&
        nA[1] >  k && nB[1] >  k && nC[1] >  k && nD[1] >  k) return 0;
    if (na[1] < -k && nb[1] < -k && nc[1] < -k && nd[1] < -k &&
        nA[1] < -k && nB[1] < -k && nC[1] < -k && nD[1] < -k) return 0;
    if (na[2] >  k && nb[2] >  k && nc[2] >  k && nd[2] >  k &&
        nA[2] >  k && nB[2] >  k && nC[2] >  k && nD[2] >  k) return 0;
    if (na[2] < -k && nb[2] < -k && nc[2] < -k && nd[2] < -k &&
        nA[2] < -k && nB[2] < -k && nC[2] < -k && nD[2] < -k) return 0;

    // Return the screen-space length of the longest edge.

    return max(length(na, nb, w, h),
               length(nc, nd, w, h),
               length(na, nc, w, h),
               length(nb, nd, w, h));
}

void face::label(const char *str, int n)
{
    double s[3];
    double t[3];
    
    s[0] = n * 0.5 / 2048;
    s[1] = n * 1.0 / 2048;
    s[2] = n * 1.0 / 2048;
    
    t[0] = (a[0] + b[0] + c[0] + d[0]) / 4;
    t[1] = (a[1] + b[1] + c[1] + d[1]) / 4;
    t[2] = (a[2] + b[2] + c[2] + d[2]) / 4;
    
    glPushMatrix();
    {
        glTranslated(t[0], t[1], t[2]);
        glScaled    (s[0], s[1], s[2]);

        for (const char *p = str; p && *p; p++)
            draw_glyph(*p);
    }
    glPopMatrix();
}

void face::draw(const double *M, int w, int h, int m)
{
    double s = evaluate(M, w, h);
    
    if (s > 0)
    {
        if (m > 0 && s > 512)
        {
            face A;
            face B;
            face C;
            face D;
            
            divide(A, B, C, D);
            
            A.draw(M, w, h, m - 1);
            B.draw(M, w, h, m - 1);
            C.draw(M, w, h, m - 1);
            D.draw(M, w, h, m - 1);
        }
        else
        {
            glColor4d(1.0, 1.0, 1.0, 0.2);

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
            
            glBegin(GL_LINE_LOOP);
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
            
            glColor4d(1.0, 1.0, 1.0, 0.5);

            static char buf[256];
            sprintf(buf, "%d", int(s));
            
            label(buf, 1 << m);
        }
    }
}

//------------------------------------------------------------------------------

void sph_model::draw(const double *P, const double *V, int w, int h)
{
    double M[16];
    
    mmultiply (M, P, V);
    
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

    for (int i = 0; i < 6; ++i)
    {
        face F;
        
        vnormalize(F.a, cube_v[cube_i[i][0]]);
        vnormalize(F.b, cube_v[cube_i[i][1]]);
        vnormalize(F.c, cube_v[cube_i[i][2]]);
        vnormalize(F.d, cube_v[cube_i[i][3]]);
        
        F.draw(M, w, h, 8);
    }
}

//------------------------------------------------------------------------------
