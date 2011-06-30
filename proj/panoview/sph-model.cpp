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
    cache(cache), depth(m), size(s), time(0), status(cube_size(m), s_halt)
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

static void bislerp(double *p, const double *a, const double *b,
                               const double *c, const double *d,
                               double x, double y)
{
    double t[3];
    double u[3];
    
    vslerp(t, a, b, x);
    vslerp(u, c, d, x);
    vslerp(p, t, u, y);
}

static double measure(int f, double r, double l, double t, double b,
                                const double *M, int w, int h)
{
    double A[3], oa[3], oA[3], na[3], nA[3];
    double B[3], ob[3], oB[3], nb[3], nB[3];
    double C[3], oc[3], oC[3], nc[3], nC[3];
    double D[3], od[3], oD[3], nd[3], nD[3];
    
    vnormalize(A, cube_v[cube_i[f][0]]);
    vnormalize(B, cube_v[cube_i[f][1]]);
    vnormalize(C, cube_v[cube_i[f][2]]);
    vnormalize(D, cube_v[cube_i[f][3]]);
    
    bislerp(oa, A, B, C, D, r, t);
    bislerp(ob, A, B, C, D, l, t);
    bislerp(oc, A, B, C, D, r, b);
    bislerp(od, A, B, C, D, l, b);
    
    // Compute the maximum extent due to bulge.

    double v[3];
    
    v[0] = oa[0] + ob[0] + oc[0] + od[0];
    v[1] = oa[1] + ob[1] + oc[1] + od[1];
    v[2] = oa[2] + ob[2] + oc[2] + od[2];
    
    double k = vlen(v) / vdot(oa, v);

    // Compute the outer corners of the bulge bound.
    
    vmul(oA, oa, k);
    vmul(oB, ob, k);
    vmul(oC, oc, k);
    vmul(oD, od, k);
    
    // Compute normalized device coordinates for all eight corners.
    
    if (!project(na, M, oa)) return 0;
    if (!project(nb, M, ob)) return 0;
    if (!project(nc, M, oc)) return 0;
    if (!project(nd, M, od)) return 0;
    if (!project(nA, M, oA)) return 0;
    if (!project(nB, M, oB)) return 0;
    if (!project(nC, M, oC)) return 0;
    if (!project(nD, M, oD)) return 0;

    // Check that any part of the boundary is visible.

    static const double d = 1;

    if (na[0] >  d && nb[0] >  d && nc[0] >  d && nd[0] >  d &&
        nA[0] >  d && nB[0] >  d && nC[0] >  d && nD[0] >  d) return 0;
    if (na[0] < -d && nb[0] < -d && nc[0] < -d && nd[0] < -d &&
        nA[0] < -d && nB[0] < -d && nC[0] < -d && nD[0] < -d) return 0;
    if (na[1] >  d && nb[1] >  d && nc[1] >  d && nd[1] >  d &&
        nA[1] >  d && nB[1] >  d && nC[1] >  d && nD[1] >  d) return 0;
    if (na[1] < -d && nb[1] < -d && nc[1] < -d && nd[1] < -d &&
        nA[1] < -d && nB[1] < -d && nC[1] < -d && nD[1] < -d) return 0;
    if (na[2] >  d && nb[2] >  d && nc[2] >  d && nd[2] >  d &&
        nA[2] >  d && nB[2] >  d && nC[2] >  d && nD[2] >  d) return 0;
    if (na[2] < -d && nb[2] < -d && nc[2] < -d && nd[2] < -d &&
        nA[2] < -d && nB[2] < -d && nC[2] < -d && nD[2] < -d) return 0;

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
        prep_face(i, i, depth, 0, 1, 0, 1, M, w, h);
    
    cache.update(time);
}

void sph_model::prep_face(int f, int i, int d,
                          double r, double l, double t, double b, 
                          const double *M, int w, int h)
{
    double s = measure(f, r, l, t, b, M, w, h);
    
    if (s > 0)
    {
        if (d > 0 && s > size)
        {
            int i0 = face_child(i, 0);
            int i1 = face_child(i, 1);
            int i2 = face_child(i, 2);
            int i3 = face_child(i, 3);

            const double x = (r + l) * 0.5;
            const double y = (t + b) * 0.5;

            prep_face(f, i0, d - 1, r, x, t, y, M, w, h);
            prep_face(f, i1, d - 1, x, l, t, y, M, w, h);
            prep_face(f, i2, d - 1, r, x, y, b, M, w, h);
            prep_face(f, i3, d - 1, x, l, y, b, M, w, h);

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

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
    
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonOffset(-4.0f, -4.0f);

    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glUseProgram(program);
    {
        glUniform1f(page_size, size);

        for (int i = 0; i < 6; ++i)
        {
            double a[3], b[3], c[3], d[3];
            
            vnormalize(a, cube_v[cube_i[i][0]]);
            vnormalize(b, cube_v[cube_i[i][1]]);
            vnormalize(c, cube_v[cube_i[i][2]]);
            vnormalize(d, cube_v[cube_i[i][3]]);
            
            glUniform3f(pos_a, GLfloat(a[0]), GLfloat(a[1]), GLfloat(a[2]));
            glUniform3f(pos_b, GLfloat(b[0]), GLfloat(b[1]), GLfloat(b[2]));
            glUniform3f(pos_c, GLfloat(c[0]), GLfloat(c[1]), GLfloat(c[2]));
            glUniform3f(pos_d, GLfloat(d[0]), GLfloat(d[1]), GLfloat(d[2]));

            glUniform1i(best_level, 0);

#if 1
            draw_face(f, i, 0, 0, 1, 0, 1, 0);
#else
            GLfloat fill[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            GLfloat line[4] = { 0.0f, 0.0f, 0.0f, 0.4f };
        
            glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, fill);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            draw_face(f, i, 0, 0, 1, 0, 1, 0);

            glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, line);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw_face(f, i, 0, 0, 1, 0, 1, 0);
#endif
        }
    }
    glUseProgram(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glDisableClientState(GL_VERTEX_ARRAY);

    glActiveTexture(GL_TEXTURE0);

    cache.draw();
    time++;
}

void sph_model::draw_face(int f, int i, int d,
                          double r, double l, double t, double b, int B)
{
    GLuint o = 0;

    if (status[i] != s_halt)
    {
        glUniform2f(tex_a[d], GLfloat(r), GLfloat(t));
        glUniform2f(tex_d[d], GLfloat(l), GLfloat(b));

        if ((o = cache.get_page(time, f, i))) B = d;

        glActiveTexture(GL_TEXTURE0 + d);
        glBindTexture(GL_TEXTURE_2D, o);
    }

    if (status[i] == s_pass)
    {
        const double x = (r + l) * 0.5;
        const double y = (t + b) * 0.5;

        draw_face(f, face_child(i, 0), d + 1, r, x, t, y, B);
        draw_face(f, face_child(i, 1), d + 1, x, l, t, y, B);
        draw_face(f, face_child(i, 2), d + 1, r, x, y, b, B);
        draw_face(f, face_child(i, 3), d + 1, x, l, y, b, B);
    }

    if (status[i] == s_draw)
    {
        int n, s, e, w, j = 0;
        
        face_neighbors(i, n, s, e, w);
        
        if (i > 5) j = (status[face_parent(n)] == s_draw ? 1 : 0)
                     | (status[face_parent(s)] == s_draw ? 2 : 0)
                     | (status[face_parent(e)] == s_draw ? 4 : 0)
                     | (status[face_parent(w)] == s_draw ? 8 : 0);

        glUniform1i(best_level, B);
        glUniform1i(this_level, d);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[j]);
        glDrawElements(GL_QUADS, count, GL_UNSIGNED_SHORT, 0);
    }
}

//------------------------------------------------------------------------------

static GLuint glGetUniformLocationv(GLuint program, const char *fmt, int d)
{
    char str[256];
    
    sprintf(str, fmt, d);
    
    return glGetUniformLocation(program, str);
}

void sph_model::init_program()
{    
    char *vert_source = load_txt("sph-render.vert");
    char *frag_source = load_txt("sph-render.frag");

    if (vert_source && frag_source)
    {
        vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_source);
        frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_source);
        program     = glsl_init_program(vert_shader, 0, frag_shader);

        glUseProgram(program);
        
        pos_a      = glGetUniformLocation(program, "pos_a");
        pos_b      = glGetUniformLocation(program, "pos_b");
        pos_c      = glGetUniformLocation(program, "pos_c");
        pos_d      = glGetUniformLocation(program, "pos_d");
        this_level = glGetUniformLocation(program, "this_level");
        best_level = glGetUniformLocation(program, "best_level");
        page_size  = glGetUniformLocation(program, "page_size");

        for (int l = 0; l < 8; ++l)
        {
            tex_a[l] = glGetUniformLocationv(program, "tex_a[%d]", l);
            tex_d[l] = glGetUniformLocationv(program, "tex_d[%d]", l);

            glUniform1i(glGetUniformLocationv(program, "texture[%d]", l), l);
        }
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
