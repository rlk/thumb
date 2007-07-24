//  Copyright (C) 2007 Robert Kooima
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

#include <iostream>
#include <iomanip>
#include <limits>
#include <queue>
#include <math.h>

#include "opengl.hpp"
#include "matrix.hpp"
#include "sphere.hpp"
#include "glob.hpp"
#include "view.hpp"

//=============================================================================

uni::sphere::sphere(uni::geodat& dat,
                    uni::georen& ren,
                    uni::geomap& color,
                    uni::geomap& normal,
                    uni::geomap& height,
                    double r0,
                    double r1, double bias, GLsizei cache) :

    count(0),

    r0(r0),
    r1(r1),

    visible(false),
    dist(0),

    bias (bias),
    cache(cache),
    frame(0),

    dat(dat),
    tex(dat.depth(), cache),
    nrm(dat.depth(), cache),
    pos(dat.depth(), cache),
    acc(dat.depth(), cache),
    ext(dat.depth(), cache),
    vtx(dat.depth(), cache),
    ren(ren),

    color(color),
    normal(normal),
    height(height)

{
    // Initialize all points.

    for (int k = 0; k < 12; ++k)
        P[k] = (new point(dat.ico()->point_v(k)))->ref();

    // Initialize all patches.

    for (int k = 0; k < 20; ++k)
        C[k] = 0;

    // Set default configuration.
    
    memset(p, 0, 3 * sizeof (double));

    n[0] = 0;
    n[1] = 1;
    n[2] = 0;
    a    = 0;
    z0   = 1.0;
    z1   = 2.0;
}

uni::sphere::~sphere()
{
    // Delete all patches and points.

    for (int k = 0; k < 20; ++k) if (C[k]) delete C[k];
    for (int k = 0; k < 12; ++k) if (P[k]) delete P[k];
}

void uni::sphere::move(double px, double py, double pz,
                       double nx, double ny, double nz, double ra)
{
    p[0] = px;
    p[1] = py;
    p[2] = pz;

    n[0] = nx;
    n[1] = ny;
    n[2] = nz;

    a    = ra;

    normalize(n);
}

void uni::sphere::turn(double dr)
{
    a += dr;
}

//-----------------------------------------------------------------------------

void uni::sphere::transform(const double *Pv,
                            const double *Mv,
                            const double *Iv)
{
    double A[16];
    double B[16];

    // Compute the planetary tilt transformation.

    load_idt(A);

    A[4] = n[0];
    A[5] = n[1];
    A[6] = n[2];

    crossprod(A + 8, A + 0, A + 4);

    load_xps(B, A);

    // Compose the object-to-camera transformation.

    load_mat(M, Iv);

    Rmul_xlt_mat(M, p[0], p[1], p[2]);  // Planet position
    mult_mat_mat(M, M, A);              // Planet tilt
    Rmul_rot_mat(M, 0, 1, 0, a);        // Planet rotation

    // Compose the object-to-camera inverse.

    load_mat(I, Mv);

    Lmul_xlt_inv(I, p[0], p[1], p[2]);  // Planet position
    mult_mat_mat(I, B, I);              // Planet tilt
    Lmul_rot_inv(I, 0, 1, 0, a);        // Planet rotation

    // Compose the model-view-projection.

    mult_mat_mat(MVP, Pv, M);
}

//-----------------------------------------------------------------------------

void uni::sphere::view(const double *Pv,
                       const double *Mv,
                       const double *Iv)
{
    if ((visible = true)) // HACK (visible = cam->test(p, r1)))
    {
        double o[ 3] = { 0, 0, 0 };
        double O[20];

        // The sphere is visible.  Cache the transform and view vector.

        transform(Pv, Mv, Iv);

        mult_mat_vec3(v, I, o);

        // Compute the horizon plane.

        V[0] = v[0];
        V[1] = v[1];
        V[2] = v[2];
        V[3] = -r0 * r0 / sqrt(DOT3(V, V));

        normalize(V);

        // Determine the view planes. TODO: move this up.

        ::view->world_frustum(O);

        mult_xps_vec4(V +  4, M, O +  0);
        mult_xps_vec4(V +  8, M, O +  4);
        mult_xps_vec4(V + 12, M, O +  8);
        mult_xps_vec4(V + 16, M, O + 12);
        mult_xps_vec4(V + 20, M, O + 16);

        // Apply the accumulated patch radii.
/*
        const GLfloat *p = ext.rmap();

        for (int k = 0; k < 20; ++k)
            if (C[k])
                C[k]->view(count, p);

        z0 = std::numeric_limits<double>::max();
        z1 = std::numeric_limits<double>::min();

        for (int i = 0; i < count; ++i)
        {
            double a = double(p[4 * i + 0]);
            double z = double(p[4 * i + 1]);

            if (a > 0) z0 = std::min(z0, a);
                       z1 = std::max(z1, z);
        }

        ext.umap();
*/
    }
    else
    {
        // The sphere is not visible.  Delete any existing patches.

        for (int k = 0; k < 20; ++k)
            if (C[k])
            {
                delete(C[k]);
                C[k] = 0;
            }
    }

    // Cache the view distance for use in depth sorting.

    dist = ::view->dist(p);
/*
    z0   = dist - 1.0;
    z1   = dist + 1.0;
*/
}

void uni::sphere::step()
{
    count = 0;

    if (visible)
    {
        // Refine the icosahedron to fit the current view.

        for (int k = 0; k < 20; ++k)
        {
            if (C[k] == 0)
            {
                const int    *i  = dat.ico()->point_i(k);

                const double *t0 = dat.ico()->patch_c(k, 0);
                const double *t1 = dat.ico()->patch_c(k, 1);
                const double *t2 = dat.ico()->patch_c(k, 2);

                C[k] = new patch(P[i[0]], t0,
                                 P[i[1]], t1,
                                 P[i[2]], t2, r0, r0, r1);
            }

            const int *i = dat.ico()->patch_i(k);
            const int *j = dat.ico()->patch_j(k);

            context ctx(C[k], C[i[0]], j[0],
                              C[i[1]], j[1],
                              C[i[2]], j[2]);

            C[k] = C[k]->step(ctx, V, v, bias, 0, count);
        }

        // Set up geometry generation.

        if ((count = std::min(count, cache)))
        {
            frame++;

            // Seed the normal and position generators.

            nrm.init();
            pos.init();
            tex.init();
            {
                for (int k = 0; k < 20; ++k)
                    if (C[k])
                        C[k]->seed(nrm, pos, tex, M, I, frame);
            }
            tex.fini(count);
            nrm.fini(count);
            pos.fini(count);

            // Store the depth range. (HACK out extrema reduction)

            z0 = pos.min_z();
            z1 = pos.max_z();
        }
    }
}

void uni::sphere::prep()
{
    if (count)
    {
        dat.lut()->bind(GL_TEXTURE1);
        dat.rad()->bind(GL_TEXTURE3);
        {
            // Generate texture coordinates.

            tex.proc(count);

            // Generate normals.

            nrm.proc(count);

            // Generate positions.

            nrm.bind(GL_TEXTURE2);
            {
                pos.proc(count);
            }
            nrm.free(GL_TEXTURE2);
        }
        dat.rad()->free(GL_TEXTURE3);
        dat.lut()->free(GL_TEXTURE1);

        // Bind all generated attribute for use in terrain accumulation.

        pos.bind(GL_TEXTURE2);
        nrm.bind(GL_TEXTURE3);
        tex.bind(GL_TEXTURE4);
        {
            // Accumulate terrain maps.  TODO: move this to geotex::proc?

            acc.init(count);
/*
            acc.bind_proc();
            {
                int w = int(dat.vtx_len());
                int h = int(count);

                height.draw(V, MVP, p, w, h);
            }
            acc.free_proc();
*/
            acc.swap();
        }
        tex.free(GL_TEXTURE4);
        nrm.free(GL_TEXTURE3);
        pos.free(GL_TEXTURE2);

        // Find the extrema of the accumulated positions.
/*
        acc.bind(GL_TEXTURE1);
        {
            ext.proc(count);
        }
        acc.free(GL_TEXTURE1);
*/
        // Copy the generated coordinates to the vertex buffer.

        tex.bind_frame();
        vtx.read_c(count);
        tex.free_frame();

        // Copy the generated normals to the vertex buffer.

        nrm.bind_frame();
        vtx.read_n(count);
        nrm.free_frame();

        // Copy the generated positions to the vertex buffer.

        acc.bind_frame();
        vtx.read_v(count);
        acc.free_frame();
    }
}

void uni::sphere::pass()
{
    GLsizei vc = dat.vtx_len();
    GLsizei tc = dat.idx_len();

    for (int k = 0; k < 20; ++k)
        if (C[k])
        {
            const int *i = dat.ico()->patch_i(k);
            const int *j = dat.ico()->patch_j(k);

            context ctx(C[k], C[i[0]], j[0],
                              C[i[1]], j[1],
                              C[i[2]], j[2]);

            C[k]->draw(ctx, cache, vc, tc);
        }
}

void uni::sphere::draw()
{
    if (count)
    {
        GLfloat L[4];
        double  t[4];
        double  a[4];
        double  A[16];

        // Position a directional lightsource at the origin.  TODO: move this?

        ::view->get_M(A);

        mult_xps_vec3(t, A, p);
        mult_xps_vec3(a, A, n);

        normalize(t);
        normalize(a);

        L[0] = GLfloat(-t[0]);
        L[1] = GLfloat(-t[1]);
        L[2] = GLfloat(-t[2]);
        L[3] = 0;

        glLightfv(GL_LIGHT0, GL_POSITION, L);

        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);
        {
            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_NORMALIZE);

            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_NORMAL_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                // Draw the texture coordinates.

                ren.cyl()->bind();
                dat.idx()->bind();
                vtx.bind();
                {
                    pass();
                }
                vtx.free();
                dat.idx()->free();
                ren.cyl()->free();

                glPushMatrix();
                {
                    glLoadMatrixd(M);

                    glEnable(GL_DEPTH_CLAMP_NV);
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);

                    glDisable(GL_DEPTH_TEST);
                    glDepthMask(GL_FALSE);

                    glDisable(GL_BLEND);
                    glEnable(GL_ALPHA_TEST);
                    glAlphaFunc(GL_GREATER, 0.5);

                    // Draw the diffuse maps.

                    ren.dif()->bind();
                    {
                        color.draw(V, v);
                    }
                    ren.dif()->free();

                    // Draw the normal maps.

                    ren.nrm()->axis(a);
                    ren.nrm()->bind();
                    {
                        normal.draw(V, v);
                    }
                    ren.nrm()->free();

                    glCullFace(GL_BACK);
                    glDisable(GL_DEPTH_CLAMP_NV);
                    glDisable(GL_STENCIL_TEST);
                    glDisable(GL_ALPHA_TEST);
                    glEnable(GL_CULL_FACE);
                    glDepthMask(GL_TRUE);
                }
                glPopMatrix();

                // Draw the illuminated geometry.

                ren.bind();
                dat.idx()->bind();
                vtx.bind();
                {
                    pass();
                }
                vtx.free();
                dat.idx()->free();
                ren.free();

                // Slap down a debug wireframe.
/*
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_DEPTH_TEST);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor4f(1.0f, 1.0f, 1.0f, 0.25f);

                dat.idx()->bind();
                vtx.bind();
                {
                    pass();
                }
                vtx.free();
                dat.idx()->free();
*/
/*
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_CULL_FACE);
                glEnable(GL_DEPTH_CLAMP_NV);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor4f(1.0f, 1.0f, 0.0f, 0.5f);
                glPushMatrix();
                {
                    glLoadMatrixd(M);
                    color.wire();
                }
                glPopMatrix();
                glDisable(GL_DEPTH_CLAMP_NV);
*/
            }
            glPopClientAttrib();
        }
        glPopAttrib();
    }
}

/*
void uni::sphere::wire()
{
    if (count)
    {
        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
        {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_COLOR_MATERIAL);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

//          glEnable(GL_TEXTURE_2D);
//          glEnable(GL_DEPTH_TEST);

            xfrm();

            glMatrixMode(GL_TEXTURE);
            {
                glPushMatrix();
                glLoadIdentity();
                glScalef(8.0f, 8.0f, 8.0f);
            }
            glMatrixMode(GL_MODELVIEW);

//          color->bind(GL_TEXTURE0);

            for (int k = 0; k < 20; ++k)
                if (C[k])
                    C[k]->wire();

//          color->free(GL_TEXTURE0);

            glMatrixMode(GL_TEXTURE);
            {
                glPopMatrix();
            }
            glMatrixMode(GL_MODELVIEW);
        }
        glPopAttrib();
    }
}
*/
void uni::sphere::xfrm()
{
    double A[16];

    load_idt(A);

    A[4] = n[0];
    A[5] = n[1];
    A[6] = n[2];

    crossprod(A + 8, A + 0, A + 4);

    glTranslated(p[0], p[1], p[2]);
    glMultMatrixd(A);
    glRotated(a, 0, 1, 0);
}

void uni::sphere::getz(double& N, double &F)
{
    N = z0;
    F = z1;
}

//-----------------------------------------------------------------------------

bool uni::sphcmp(const sphere *A, const sphere *B)
{
    return (A->distance() < B->distance());
}

//-----------------------------------------------------------------------------
