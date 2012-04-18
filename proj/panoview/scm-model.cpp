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
#include <cassert>
#include <limits>

#include "math3d.h"
#include "glsl.h"

#include "scm-model.hpp"
#include "scm-index.hpp"

//------------------------------------------------------------------------------

#if 0
typedef GLushort         GLindex;
#define GL_ELEMENT_INDEX GL_UNSIGNED_SHORT
#else
typedef GLuint           GLindex;
#define GL_ELEMENT_INDEX GL_UNSIGNED_INT
#endif

//------------------------------------------------------------------------------

static inline double scale(double k, double t)
{
    if (k < 1.0)
        return std::min(t / k, 1.0 - (1.0 - t) * k);
    else
        return std::max(t / k, 1.0 - (1.0 - t) * k);
}

void scm_model::zoom(double *w, const double *v)
{
    double d = vdot(v, zoomv);

    if (-1 < d && d < 1)
    {
        double b = scale(zoomk, acos(d) / M_PI) * M_PI;

        double y[3];
        double x[3];

        vcrs(y, v, zoomv);
        vnormalize(y, y);
        vcrs(x, zoomv, y);
        vnormalize(x, x);

        vmul(w, zoomv, cos(b));
        vmad(w, w,  x, sin(b));
    }
    else vcpy(w, v);
}

//------------------------------------------------------------------------------

scm_model::scm_model(scm_cache& cache,
                     const char *vert,
                     const char *frag, int n, int s) :
    cache(cache),
    time(1),
    size(s),
    debug(false)
{
    init_program(vert, frag);
    init_arrays(n);

    zoomv[0] =  0;
    zoomv[1] =  0;
    zoomv[2] = -1;
    zoomk    =  1;
}

scm_model::~scm_model()
{
    free_arrays();
    free_program();
}

GLfloat scm_model::age(int then)
{
    GLfloat a = GLfloat(time - then) / 60.f;
    return (a > 1.f) ? 1.f : a;
}

//------------------------------------------------------------------------------

static inline double length(const double *a, const double *b, int w, int h)
{
    if (a[3] <= 0 && b[3] <= 0) return 0;
    if (a[3] <= 0)              return HUGE_VAL;
    if (b[3] <= 0)              return HUGE_VAL;

    double dx = (a[0] / a[3] - b[0] / b[3]) * w / 2;
    double dy = (a[1] / a[3] - b[1] / b[3]) * h / 2;

    return sqrt(dx * dx + dy * dy);
}

double scm_model::view_page(const double *M, int vw, int vh,
                            double r0, double r1, long long i)
{
    // Compute the corner vectors of the zoomed page.

    double v[12];

    scm_page_corners(i, v);

#if 0
    if (zoomk != 1)
    {
        zoom(v + 0, v + 0);
        zoom(v + 3, v + 3);
        zoom(v + 6, v + 6);
        zoom(v + 9, v + 9);
    }
#endif

    // Compute the maximum extent due to bulge.

    double u[3];

    u[0] = v[0] + v[3] + v[6] + v[ 9];
    u[1] = v[1] + v[4] + v[7] + v[10];
    u[2] = v[2] + v[5] + v[8] + v[11];

    double r2 = r1 * vlen(u) / vdot(v, u);

    // Apply the inner and outer radii to the bounding volume.

    double a[3], e[3], A[4], E[4];
    double b[3], f[3], B[4], F[4];
    double c[3], g[3], C[4], G[4];
    double d[3], h[3], D[4], H[4];

    vmul(a, v + 0, r0);
    vmul(b, v + 3, r0);
    vmul(c, v + 6, r0);
    vmul(d, v + 9, r0);

    vmul(e, v + 0, r2);
    vmul(f, v + 3, r2);
    vmul(g, v + 6, r2);
    vmul(h, v + 9, r2);

    // Compute W and reject any volume on the far side of the singularity.

    A[3] = M[ 3] * a[0] + M[ 7] * a[1] + M[11] * a[2] + M[15];
    B[3] = M[ 3] * b[0] + M[ 7] * b[1] + M[11] * b[2] + M[15];
    C[3] = M[ 3] * c[0] + M[ 7] * c[1] + M[11] * c[2] + M[15];
    D[3] = M[ 3] * d[0] + M[ 7] * d[1] + M[11] * d[2] + M[15];
    E[3] = M[ 3] * e[0] + M[ 7] * e[1] + M[11] * e[2] + M[15];
    F[3] = M[ 3] * f[0] + M[ 7] * f[1] + M[11] * f[2] + M[15];
    G[3] = M[ 3] * g[0] + M[ 7] * g[1] + M[11] * g[2] + M[15];
    H[3] = M[ 3] * h[0] + M[ 7] * h[1] + M[11] * h[2] + M[15];

    if (A[3] <= 0 && B[3] <= 0 && C[3] <= 0 && D[3] <= 0 &&
        E[3] <= 0 && F[3] <= 0 && G[3] <= 0 && H[3] <= 0)
        return 0;

    // Compute Z and trivially reject using the near and far clipping planes.

    A[2] = M[ 2] * a[0] + M[ 6] * a[1] + M[10] * a[2] + M[14];
    B[2] = M[ 2] * b[0] + M[ 6] * b[1] + M[10] * b[2] + M[14];
    C[2] = M[ 2] * c[0] + M[ 6] * c[1] + M[10] * c[2] + M[14];
    D[2] = M[ 2] * d[0] + M[ 6] * d[1] + M[10] * d[2] + M[14];
    E[2] = M[ 2] * e[0] + M[ 6] * e[1] + M[10] * e[2] + M[14];
    F[2] = M[ 2] * f[0] + M[ 6] * f[1] + M[10] * f[2] + M[14];
    G[2] = M[ 2] * g[0] + M[ 6] * g[1] + M[10] * g[2] + M[14];
    H[2] = M[ 2] * h[0] + M[ 6] * h[1] + M[10] * h[2] + M[14];

    if (A[2] >  A[3] && B[2] >  B[3] && C[2] >  C[3] && D[2] >  D[3] &&
        E[2] >  E[3] && F[2] >  F[3] && G[2] >  G[3] && H[2] >  H[3])
        return 0;
    if (A[2] < -A[3] && B[2] < -B[3] && C[2] < -C[3] && D[2] < -D[3] &&
        E[2] < -E[3] && F[2] < -F[3] && G[2] < -G[3] && H[2] < -H[3])
        return 0;

    // Compute Y and trivially reject using the bottom and top clipping planes.

    A[1] = M[ 1] * a[0] + M[ 5] * a[1] + M[ 9] * a[2] + M[13];
    B[1] = M[ 1] * b[0] + M[ 5] * b[1] + M[ 9] * b[2] + M[13];
    C[1] = M[ 1] * c[0] + M[ 5] * c[1] + M[ 9] * c[2] + M[13];
    D[1] = M[ 1] * d[0] + M[ 5] * d[1] + M[ 9] * d[2] + M[13];
    E[1] = M[ 1] * e[0] + M[ 5] * e[1] + M[ 9] * e[2] + M[13];
    F[1] = M[ 1] * f[0] + M[ 5] * f[1] + M[ 9] * f[2] + M[13];
    G[1] = M[ 1] * g[0] + M[ 5] * g[1] + M[ 9] * g[2] + M[13];
    H[1] = M[ 1] * h[0] + M[ 5] * h[1] + M[ 9] * h[2] + M[13];

    if (A[1] >  A[3] && B[1] >  B[3] && C[1] >  C[3] && D[1] >  D[3] &&
        E[1] >  E[3] && F[1] >  F[3] && G[1] >  G[3] && H[1] >  H[3])
        return 0;
    if (A[1] < -A[3] && B[1] < -B[3] && C[1] < -C[3] && D[1] < -D[3] &&
        E[1] < -E[3] && F[1] < -F[3] && G[1] < -G[3] && H[1] < -H[3])
        return 0;

    // Compute X and trivially reject using the left and right clipping planes.

    A[0] = M[ 0] * a[0] + M[ 4] * a[1] + M[ 8] * a[2] + M[12];
    B[0] = M[ 0] * b[0] + M[ 4] * b[1] + M[ 8] * b[2] + M[12];
    C[0] = M[ 0] * c[0] + M[ 4] * c[1] + M[ 8] * c[2] + M[12];
    D[0] = M[ 0] * d[0] + M[ 4] * d[1] + M[ 8] * d[2] + M[12];
    E[0] = M[ 0] * e[0] + M[ 4] * e[1] + M[ 8] * e[2] + M[12];
    F[0] = M[ 0] * f[0] + M[ 4] * f[1] + M[ 8] * f[2] + M[12];
    G[0] = M[ 0] * g[0] + M[ 4] * g[1] + M[ 8] * g[2] + M[12];
    H[0] = M[ 0] * h[0] + M[ 4] * h[1] + M[ 8] * h[2] + M[12];

    if (A[0] >  A[3] && B[0] >  B[3] && C[0] >  C[3] && D[0] >  D[3] &&
        E[0] >  E[3] && F[0] >  F[3] && G[0] >  G[3] && H[0] >  H[3])
        return 0;
    if (A[0] < -A[3] && B[0] < -B[3] && C[0] < -C[3] && D[0] < -D[3] &&
        E[0] < -E[3] && F[0] < -F[3] && G[0] < -G[3] && H[0] < -H[3])
        return 0;

    if (debug)
    {
        glVertex3dv(e); glVertex3dv(f);
        glVertex3dv(g); glVertex3dv(h);
        glVertex3dv(e); glVertex3dv(g);
        glVertex3dv(f); glVertex3dv(h);
    }

    // Compute the length of the longest visible edge, in pixels.

    // return std::max(std::max(std::max(length(A, B, vw, vh),
    //                                   length(C, D, vw, vh)),
    //                          std::max(length(A, C, vw, vh),
    //                                   length(B, D, vw, vh))),
    //                 std::max(std::max(length(E, F, vw, vh),
    //                                   length(G, H, vw, vh)),
    //                          std::max(length(E, G, vw, vh),
    //                                   length(F, H, vw, vh))));

    return std::max(std::max(length(A, B, vw, vh),
                             length(C, D, vw, vh)),
                    std::max(length(A, C, vw, vh),
                             length(B, D, vw, vh)));
}

double scm_model::test_page(const double *M,  int w, int h,
                               const int *vv, int vc, long long i)
{
    // Merge the bounds of this page in each vertex data set.

    float r0 = 1.0;
    float r1 = 1.0;

    cache.page_bounds(i, vv, vc, r0, r1);

    // Compute and test the on-screen pixel size of this bounds.

    return view_page(M, w, h, double(r0), double(r1), i);
}

//------------------------------------------------------------------------------

void scm_model::add_page(const double *M,  int w,  int h,
                            const int *vv, int vc, long long i)
{
    if (!is_set(i))
    {
        double k = test_page(M, w, h, vv, vc, i);

        if (k > 0)
        {
            pages.insert(i);

            if (i > 5)
            {
                long long p = scm_page_parent(i);

                add_page(M, w, h, vv, vc, p);

                switch (scm_page_order(i))
                {
                    case 0:
                        add_page(M, w, h, vv, vc, scm_page_north(p));
                        add_page(M, w, h, vv, vc, scm_page_south(i));
                        add_page(M, w, h, vv, vc, scm_page_east (i));
                        add_page(M, w, h, vv, vc, scm_page_west (p));
                        break;
                    case 1:
                        add_page(M, w, h, vv, vc, scm_page_north(p));
                        add_page(M, w, h, vv, vc, scm_page_south(i));
                        add_page(M, w, h, vv, vc, scm_page_east (p));
                        add_page(M, w, h, vv, vc, scm_page_west (i));
                        break;
                    case 2:
                        add_page(M, w, h, vv, vc, scm_page_north(i));
                        add_page(M, w, h, vv, vc, scm_page_south(p));
                        add_page(M, w, h, vv, vc, scm_page_east (i));
                        add_page(M, w, h, vv, vc, scm_page_west (p));
                        break;
                    case 3:
                        add_page(M, w, h, vv, vc, scm_page_north(i));
                        add_page(M, w, h, vv, vc, scm_page_south(p));
                        add_page(M, w, h, vv, vc, scm_page_east (p));
                        add_page(M, w, h, vv, vc, scm_page_west (i));
                        break;
                }
            }
        }
    }
}

bool scm_model::prep_page(const double *M,  int w,  int h,
                             const int *vv, int vc,
                             const int *fv, int fc, long long i)
{
    // If this page is missing from all data sets, skip it.

    if (cache.page_status(i, vv, vc, fv, fc))
    {
        // Compute the on-screen pixel size of this page.

        double k = test_page(M, w, h, vv, vc, i);

        // Subdivide if too large, otherwise mark for drawing.

        if (k > 0)
        {
            if (k > size)
            {
                long long i0 = scm_page_child(i, 0);
                long long i1 = scm_page_child(i, 1);
                long long i2 = scm_page_child(i, 2);
                long long i3 = scm_page_child(i, 3);

                bool b0 = prep_page(M, w, h, vv, vc, fv, fc, i0);
                bool b1 = prep_page(M, w, h, vv, vc, fv, fc, i1);
                bool b2 = prep_page(M, w, h, vv, vc, fv, fc, i2);
                bool b3 = prep_page(M, w, h, vv, vc, fv, fc, i3);

                if (b0 || b1 || b2 || b3)
                    return true;
            }
            add_page(M, w, h, vv, vc, i);
            return true;
        }
    }
    return false;
}

void scm_model::draw_page(const int *vv, int vc,
                          const int *fv, int fc,
                          const int *pv, int pc, int d, int f, long long i)
{
    int then = time;
    int T = vc + fc;
    int t = 0;

    // Bind and set vertex shader textures and uniforms.

    for (int vi = 0; vi < vc; ++vi, ++t)
    {
        glActiveTexture(GL_TEXTURE0 + d * T + t);
        glBindTexture(GL_TEXTURE_2D, cache.get_page(vv[vi], i, time, then));
        glUniform1f(u_v_age[d], 1.0f); // age(then)); // HACK
        glUniform1i(u_v_img[d], d * T + t);
    }

    // Bind and set fragment shader textures and uniforms.

    for (int fi = 0; fi < fc; ++fi, ++t)
    {
        glActiveTexture(GL_TEXTURE0 + d * T + t);
        glBindTexture(GL_TEXTURE_2D, cache.get_page(fv[fi], i, time, then));
        glUniform1f(u_f_age[d], age(then));
        glUniform1i(u_f_img[d], d * T + t);
    }

    // Set the texture coordinate uniforms.

    long long r = scm_page_row(i);
    long long c = scm_page_col(i);
    long long n = 1LL << d;

    glUniform2f(u_tex_a[d], GLfloat(c    ) / n, GLfloat(r    ) / n);
    glUniform2f(u_tex_d[d], GLfloat(c + 1) / n, GLfloat(r + 1) / n);

    // Won't someone please think of the children?

    long long i0 = scm_page_child(i, 0);
    long long i1 = scm_page_child(i, 1);
    long long i2 = scm_page_child(i, 2);
    long long i3 = scm_page_child(i, 3);

    bool b0 = is_set(i0);
    bool b1 = is_set(i1);
    bool b2 = is_set(i2);
    bool b3 = is_set(i3);

    if (b0 || b1 || b2 || b3)
    {
        // Draw any children marked for drawing.

        if (b0) draw_page(vv, vc, fv, fc, pv, pc, d + 1, f, i0);
        if (b1) draw_page(vv, vc, fv, fc, pv, pc, d + 1, f, i1);
        if (b2) draw_page(vv, vc, fv, fc, pv, pc, d + 1, f, i2);
        if (b3) draw_page(vv, vc, fv, fc, pv, pc, d + 1, f, i3);
    }
    else
    {
        // Draw this page. Select a mesh that matches up with the neighbors.

        int j = (i < 6) ? 0 : (is_set(scm_page_north(i)) ? 0 : 1)
                            | (is_set(scm_page_south(i)) ? 0 : 2)
                            | (is_set(scm_page_west (i)) ? 0 : 4)
                            | (is_set(scm_page_east (i)) ? 0 : 8);

        glUniform1i(u_level, d);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements[j]);
        glDrawElements(GL_QUADS, count, GL_ELEMENT_INDEX, 0);
    }

    // Undo any texture bindings.

    for (int t = 0; t < T; ++t)
    {
        glActiveTexture(GL_TEXTURE0 + d * T + t);
        glBindTexture(GL_TEXTURE_2D, cache.get_fill());
    }
}

//------------------------------------------------------------------------------

void scm_model::prep(const double *P, const double *V, int w, int h,
                                               const int *vv, int vc,
                                               const int *fv, int fc)
{
    double M[16];

    mmultiply(M, P, V);

    pages.clear();

    if (debug)
    {
        glUseProgram(0);
        glBegin(GL_LINES);
    }

    prep_page(M, w, h, vv, vc, fv, fc, 0);
    prep_page(M, w, h, vv, vc, fv, fc, 1);
    prep_page(M, w, h, vv, vc, fv, fc, 2);
    prep_page(M, w, h, vv, vc, fv, fc, 3);
    prep_page(M, w, h, vv, vc, fv, fc, 4);
    prep_page(M, w, h, vv, vc, fv, fc, 5);

    if (debug)
    {
        glEnd();
    }

    std::set<long long>::iterator i;

    GLuint o;
    int then;

    for (i = pages.begin(); i != pages.end(); ++i)
    {
        for (int vi = 0; vi < vc; ++vi)
            o = cache.get_page(vv[vi], *i, time, then);
        for (int fi = 0; fi < fc; ++fi)
            o = cache.get_page(fv[fi], *i, time, then);
    }
}

void scm_model::draw(const double *P, const double *V, int w, int h,
                                               const int *vv, int vc,
                                               const int *fv, int fc,
                                               const int *pv, int pc)
{
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);

    glEnable(GL_COLOR_MATERIAL);

    prep(P, V, w, h, vv, vc, fv, fc);

    glBindBuffer(GL_ARRAY_BUFFER, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, 0);

    glUseProgram(program);
    {
        static const GLfloat faceM[6][9] = {
            {  0.f,  0.f,  1.f,  0.f,  1.f,  0.f, -1.f,  0.f,  0.f },
            {  0.f,  0.f, -1.f,  0.f,  1.f,  0.f,  1.f,  0.f,  0.f },
            {  1.f,  0.f,  0.f,  0.f,  0.f,  1.f,  0.f, -1.f,  0.f },
            {  1.f,  0.f,  0.f,  0.f,  0.f, -1.f,  0.f,  1.f,  0.f },
            {  1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f,  1.f },
            { -1.f,  0.f,  0.f,  0.f,  1.f,  0.f,  0.f,  0.f, -1.f },
        };

        glUniform1f(u_zoomk, GLfloat(zoomk));
        glUniform3f(u_zoomv, GLfloat(zoomv[0]),
                             GLfloat(zoomv[1]),
                             GLfloat(zoomv[2]));

        if (is_set(0))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[0]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 0, 0);
        }
        if (is_set(1))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[1]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 1, 1);
        }
        if (is_set(2))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[2]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 2, 2);
        }
        if (is_set(3))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[3]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 3, 3);
        }
        if (is_set(4))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[4]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 4, 4);
        }
        if (is_set(5))
        {
            glUniformMatrix3fv(u_faceM, 1, GL_TRUE, faceM[5]);
            draw_page(vv, vc, fv, fc, pv, pc, 0, 5, 5);
        }
    }
    glUseProgram(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glDisableClientState(GL_VERTEX_ARRAY);

    glActiveTexture(GL_TEXTURE0);
}

//------------------------------------------------------------------------------

void scm_model::set_fade(double k)
{
    double t = k * k * (3.0 - 2.0 * k);
    glUseProgram(program);
    glUniform1f(u_fader, GLfloat(t));
}

static GLuint glGetUniformLocationv(GLuint program, const char *fmt, int d)
{
    char str[256];
    sprintf(str, fmt, d);
    return glGetUniformLocation(program, str);
}

void scm_model::init_program(const char *vert_src,
                             const char *frag_src)
{
    if (vert_src && frag_src)
    {
        vert_shader = glsl_init_shader(GL_VERTEX_SHADER,   vert_src);
        frag_shader = glsl_init_shader(GL_FRAGMENT_SHADER, frag_src);
        program     = glsl_init_program(vert_shader, frag_shader);

        glUseProgram(program);

        u_level = glGetUniformLocation(program, "level");
        u_fader = glGetUniformLocation(program, "fader");
        u_zoomk = glGetUniformLocation(program, "zoomk");
        u_zoomv = glGetUniformLocation(program, "zoomv");
        u_faceM = glGetUniformLocation(program, "faceM");

        for (int d = 0; d < max_texture_image_units; ++d)
        {
            u_tex_a.push_back(glGetUniformLocationv(program, "tex_a[%d]", d));
            u_tex_d.push_back(glGetUniformLocationv(program, "tex_d[%d]", d));
            u_v_age.push_back(glGetUniformLocationv(program, "v_age[%d]", d));
            u_f_age.push_back(glGetUniformLocationv(program, "f_age[%d]", d));
            u_v_img.push_back(glGetUniformLocationv(program, "v_img[%d]", d));
            u_f_img.push_back(glGetUniformLocationv(program, "f_img[%d]", d));
        }
    }
}

void scm_model::free_program()
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
        GLindex a;
        GLindex b;
        GLindex d;
        GLindex c;
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
                e->a = GLindex(d * (r    ) + (c    ));
                e->b = GLindex(d * (r    ) + (c + 1));
                e->c = GLindex(d * (r + 1) + (c    ));
                e->d = GLindex(d * (r + 1) + (c + 1));
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

void scm_model::init_arrays(int n)
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

void scm_model::free_arrays()
{
    glDeleteBuffers(16, elements);
    glDeleteBuffers(1, &vertices);
}

//------------------------------------------------------------------------------
