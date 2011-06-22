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
#include <cstdio>
#include <cstdlib>

#include "math3d.h"
#include "glyph.h"
#include "glsl.h"
#include "cube.hpp"

#include "sph-model.hpp"

//------------------------------------------------------------------------------

sph_model::sph_model(sph_cache& cache, int n, int m, int s) :
    cache(cache), depth(m), size(s), status(cube_size(m), s_halt)
{
    init_program();
    init_arrays(n);
}

sph_model::~sph_model()
{
    free_arrays();
    free_program();
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

void sph_model::face::divide(face& A, face& B, face &C, face &D)
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

double sph_model::face::measure(const double *M, int w, int h)
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

//------------------------------------------------------------------------------

void sph_model::prep(const double *P, const double *V, int w, int h)
{
    double M[16];
    
    mmultiply(M, P, V);
    
    for (int i = 0; i < 6; ++i)
    {
        face F;
        
        vnormalize(F.a, cube_v[cube_i[i][0]]);
        vnormalize(F.b, cube_v[cube_i[i][1]]);
        vnormalize(F.c, cube_v[cube_i[i][2]]);
        vnormalize(F.d, cube_v[cube_i[i][3]]);
        
        prep_face(F, M, i, w, h, depth);
    }
}

void sph_model::prep_face(face& F, const double *M, int i, int w, int h, int m)
{
    double s = F.measure(M, w, h);
    
    if (s > 0)
    {
        if (m > 0 && s > size)
        {
            int i0 = face_child(i, 0);
            int i1 = face_child(i, 1);
            int i2 = face_child(i, 2);
            int i3 = face_child(i, 3);
            
            face A;
            face B;
            face C;
            face D;
            
            F.divide(A, B, C, D);
            
            prep_face(A, M, i0, w, h, m - 1);
            prep_face(B, M, i1, w, h, m - 1);
            prep_face(C, M, i2, w, h, m - 1);
            prep_face(D, M, i3, w, h, m - 1);

            if (status[i0] == s_halt && status[i1] == s_halt &&
                status[i2] == s_halt && status[i3] == s_halt)

                status[i] = s_halt;
            else
                status[i] = s_pass;
        }
        else status[i] = s_draw;
    }
    else status[i] = s_halt;
}

//------------------------------------------------------------------------------

void sph_model::draw(const double *P, const double *V, int f)
{
    double M[16];
    
    mmultiply(M, P, V);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    
    glFrontFace(GL_CW);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertices);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, 0);

        for (int i = 0; i < 6; ++i)
        {
            face F;
            
            vnormalize(F.a, cube_v[cube_i[i][0]]);
            vnormalize(F.b, cube_v[cube_i[i][1]]);
            vnormalize(F.c, cube_v[cube_i[i][2]]);
            vnormalize(F.d, cube_v[cube_i[i][3]]);
            
            draw_face(F, M, i, 0, f);
        }
    }
    glUseProgram(0);
}

void sph_model::draw_face(face& F, const double *M, int i, int l, int f)
{
    if (status[i] == s_pass)
    {
        face A;
        face B;
        face C;
        face D;
        
        F.divide(A, B, C, D);
        
        draw_face(A, M, face_child(i, 0), l + 1, f);
        draw_face(B, M, face_child(i, 1), l + 1, f);
        draw_face(C, M, face_child(i, 2), l + 1, f);
        draw_face(D, M, face_child(i, 3), l + 1, f);
    }

    if (status[i] == s_draw)
    {
        int b = 0;
        int u;
        int d;
        int r;
        int l;
        
        face_neighbors(i, u, d, r, l);
        
        if (i > 5)
            b = (status[face_parent(u)] == s_draw ? 1 : 0)
              | (status[face_parent(d)] == s_draw ? 2 : 0)
              | (status[face_parent(r)] == s_draw ? 4 : 0)
              | (status[face_parent(l)] == s_draw ? 8 : 0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[b]);
        glBindTexture(GL_TEXTURE_2D, cache.get_page(f, i));

        glUniform3f(corner_a, F.a[0], F.a[1], F.a[2]);
        glUniform3f(corner_b, F.b[0], F.b[1], F.b[2]);
        glUniform3f(corner_c, F.c[0], F.c[1], F.c[2]);
        glUniform3f(corner_d, F.d[0], F.d[1], F.d[2]);
        glUniform1f(level, GLfloat(l));
        
        glDrawElements(GL_QUADS, count, GL_UNSIGNED_SHORT, 0);
    }
}

//------------------------------------------------------------------------------

void sph_model::init_program()
{    
    char *vert_source = load_txt("sph-render.vert");
    char *frag_source = load_txt("sph-render.frag");
    
    if (vert_source && frag_source)
    {
        vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_source);
        frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_source);
        program     = glsl_init_program(vert_shader, frag_shader);
        
        glUseProgram(program);      
        
        corner_a = glGetUniformLocation(program, "corner_a");
        corner_b = glGetUniformLocation(program, "corner_b");
        corner_c = glGetUniformLocation(program, "corner_c");
        corner_d = glGetUniformLocation(program, "corner_d");
        level    = glGetUniformLocation(program, "level");
        
        glUniform1i(glGetUniformLocation(program, "texture"), 0);
        glUniform1f(glGetUniformLocation(program, "size"), size);
    }
        
    free(frag_source);
    free(vert_source);
}

void sph_model::free_program()
{
    glDeleteProgram(program);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);
}

//------------------------------------------------------------------------------

static inline int up(int d)
{
    return (d % 2) ? d + 1 : d;
}

static inline int dn(int d)
{
    return (d % 2) ? d - 1 : d;
}

static void init_vertices(int n)
{
    struct vertex
    {
        GLfloat x;
        GLfloat y;
    };
    
    const size_t s = (n + 1) * (n + 1) * sizeof (vertex);
    
    if (vertex *p = (vertex *) malloc(s))
    {
        vertex *v = p;
        
        // Compute the position of each vertex.
        
        for     (int r = 0; r <= n; ++r)
            for (int c = 0; c <= n; ++c, ++v)
            {
                v->x = GLfloat(c) / GLfloat(n);
                v->y = GLfloat(r) / GLfloat(n);
            }
        
        // Upload the vertices to the vertex buffer.
        
        glBufferData(GL_ARRAY_BUFFER, s, p, GL_STATIC_DRAW); 
        free(p);
    }
}

static void init_elements(int n, int b)
{
    struct element
    {
        GLshort a;
        GLshort b;
        GLshort d;
        GLshort c;
    };
    
    const size_t s = n * n * sizeof (element);
    const int    d = n + 1;
    
    if (element *p = (element *) malloc(s))
    {
        element *e = p;
        
        // Compute the indices for each quad.
        
        for     (int r = 0; r < n; ++r)
            for (int c = 0; c < n; ++c, ++e)
            {
                e->a = GLshort(d * (r    ) + (c    ));
                e->b = GLshort(d * (r    ) + (c + 1));
                e->c = GLshort(d * (r + 1) + (c    ));
                e->d = GLshort(d * (r + 1) + (c + 1));
            }

        // Rewind the indices to reduce edge resolution as necessary.

        element *N = p;
        element *W = p + (n - 1);
        element *E = p;
        element *S = p + (n - 1) * n;

        for (int i = 0; i < n; ++i, N += 1, S += 1, E += n, W += n)
        {
            if (b & 1) { if (i & 1) N->a -= 1; else N->b -= 1; }
            if (b & 2) { if (i & 1) S->c += 1; else S->d += 1; }
            if (b & 4) { if (i & 1) E->a += d; else E->c += d; }
            if (b & 8) { if (i & 1) W->b -= d; else W->d -= d; }
        }
        
        // Upload the indices to the element buffer.
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, s, p, GL_STATIC_DRAW);
        free(p);
    }
}

void sph_model::init_arrays(int n)
{
    glGenBuffers(1, &vertices);
    glGenBuffers(16, elements);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    init_vertices(n);
    
    for (int b = 0; b < 16; ++b)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[b]);
        init_elements(n, b);
    }
    
    count = 4 * n * n;
}

void sph_model::free_arrays()
{
    glDeleteBuffers(16, elements);
    glDeleteBuffers(1, &vertices);
}

//------------------------------------------------------------------------------
