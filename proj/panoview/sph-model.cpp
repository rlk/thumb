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
    
    void draw(int);
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

void face::draw(int l)
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
        
        A.draw(l - 1);
        B.draw(l - 1);
        C.draw(l - 1);
        D.draw(l - 1);
    }
}

//------------------------------------------------------------------------------

void sph_model::draw(int buffer_w, int buffer_h)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

//  glEnable(GL_CULL_FACE);
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
        F.draw(4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        F.draw(4);
    }
}

//------------------------------------------------------------------------------
