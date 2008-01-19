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
#include "conf.hpp"
#include "user.hpp"
#include "util.hpp"

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
    a0(r0 * 1.000),
    a1(a0 * 1.025),

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
    height(height),

    atmo_in (glob->load_program("glsl/SkyFromAtmosphere.vert",
                                "glsl/SkyFromAtmosphere.frag")),
    atmo_out(glob->load_program("glsl/SkyFromSpace.vert",
                                "glsl/SkyFromSpace.frag")),
    land_in (glob->load_program("glsl/GroundFromAtmosphere.vert",
                                "glsl/GroundFromAtmosphere.frag")),
    land_out(glob->load_program("glsl/GroundFromSpace.vert",
                                "glsl/GroundFromSpace.frag"))
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
    d0   = 1.0;
    d1   = 2.0;

    // Initialize atmosphere rendering.

    atmo_pool = glob->new_pool();
    atmo_node = new ogl::node();
    atmo_unit = new ogl::unit("solid/inverted_sphere.obj");

    atmo_pool->add_node(atmo_node);
    atmo_node->add_unit(atmo_unit);

    double S[16];
    double I[16];

    load_scl_mat(S, a1, a1, a1);
    load_scl_inv(I, a1, a1, a1);

    atmo_unit->transform(S, I);
}

uni::sphere::~sphere()
{
    glob->free_program(land_out);
    glob->free_program(land_in);
    glob->free_program(atmo_out);
    glob->free_program(atmo_in);

    if (atmo_unit) delete atmo_unit;
    if (atmo_pool) delete atmo_pool;

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

void uni::sphere::atmo_prep(const ogl::program *P) const
{
    P->bind();
    {
        double r = sqrt(DOT3(v, v));

        P->uniform("eye_to_object_mat", I, false);
        P->uniform("eye_to_object_inv", M, true);

        double R = 0.650;
        double G = 0.570;
        double B = 0.475;

        P->uniform("v3InvWavelength", 1.0 / pow(R, 4.0),
                                      1.0 / pow(G, 4.0),
                                      1.0 / pow(B, 4.0));

        P->uniform("fCameraHeight",  r);
        P->uniform("fCameraHeight2", r * r);

        P->uniform("fInnerRadius",  a0);
        P->uniform("fInnerRadius2", a0 * a0);
        P->uniform("fOuterRadius",  a1);
        P->uniform("fOuterRadius2", a1 * a1);

        double Kr = 0.0025;
        double Km = 0.0010;

        P->uniform("fKrESun", Kr * 20.0);
        P->uniform("fKmESun", Km * 20.0);
        P->uniform("fKr4PI",  Kr *  4.0 * PI);
        P->uniform("fKm4PI",  Km *  4.0 * PI);

        double sd = 0.25;

        P->uniform("fScale",                (1.0f / (a1 - a0))     );
        P->uniform("fScaleDepth",                                sd);
        P->uniform("fScaleOverScaleDepth",  (1.0f / (a1 - a0)) / sd);

        double g = -0.990;

        P->uniform("g",  g);
        P->uniform("g2", g * g);
    }
    P->free();
}

//-----------------------------------------------------------------------------

void uni::sphere::transform(const double *Mv,
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

    // Apply the transform to the atmosphere.

    if (atmo_node) atmo_node->transform(M);
}

//-----------------------------------------------------------------------------

void uni::sphere::view(const double *Mv,
                       const double *Iv,
                       const double *F, int n)
{
    if ((visible = true)) // HACK (visible = cam->test(p, r1)))
    {
        double o[3] = { 0, 0, 0 };

        // The sphere is visible.  Cache the transform and view vector.

        transform(Mv, Iv);

        mult_mat_vec3(v, I, o);

        // Compute the horizon plane.

        V[0] = v[0];
        V[1] = v[1];
        V[2] = v[2];
        V[3] = -r0 * r0 / sqrt(DOT3(V, V));

        normalize(V);
/*
        const double *P = ::view->get_plane();
        V[0] = P[0];
        V[1] = P[1];
        V[2] = P[2];
        V[3] = P[3];
*/
        // Determine the world-space view frustum planes.

        mult_xps_vec4(V +  4, M, F +  0);
        mult_xps_vec4(V +  8, M, F +  4);
        mult_xps_vec4(V + 12, M, F +  8);
        mult_xps_vec4(V + 16, M, F + 12);
    /*
    printf("%d\n", n);

    for (int k = 1; k <= n; ++k)
        printf("%+8.3f %+8.3f %+8.3f %f\n",
               V[4 * k + 0],
               V[4 * k + 1],
               V[4 * k + 2],
               V[4 * k + 3]);
    */

        // Prep the atmosphere model.

        atmo_pool->prep();
        atmo_pool->view(1, 0, 0);
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

    dist = ::user->dist(p);
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

            d0 = pos.min_d();
            d1 = pos.max_d();
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
            acc.bind_proc();
            {
                int w = int(dat.vtx_len());
                int h = int(count);

                height.draw(V, v, w, h);
            }
            acc.free_proc();
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

void uni::sphere::draw(const double *frag_d,
                       const double *frag_k)
{
    bool in = (sqrt(DOT3(v, v)) < a1);

    const ogl::program *atmo_prog = in ? atmo_in : atmo_out;
    const ogl::program *land_prog = in ? land_in : land_out;

    // Position a directional lightsource at the origin.  TODO: move this?

    GLfloat L[4];
    double  t[4];
    double  a[4];
    double  A[16];

    ::user->get_M(A);

    mult_xps_vec3(t, A, p);
    mult_xps_vec3(a, A, n);

    normalize(t);
    normalize(a);

    L[0] = GLfloat(-t[0]);
    L[1] = GLfloat(-t[1]);
    L[2] = GLfloat(-t[2]);
    L[3] = 0;

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    atmo_prep(atmo_prog);
    atmo_prep(land_prog);

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);
    {
        if (count)
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

                    // Enable rendering of the BACK of the view volumes.

                    glEnable(GL_DEPTH_CLAMP_NV);
                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_FRONT);

                    // Alpha test discards texels outside of texture borders.

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

                    // Revert the state.

                    glCullFace(GL_BACK);
                    glDisable(GL_ALPHA_TEST);
                    glDisable(GL_DEPTH_CLAMP_NV);
                }
                glPopMatrix();

                // Draw the illuminated geometry.

                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);

                land_prog->bind();
                {
                    land_prog->uniform("dif", 1);
                    land_prog->uniform("nrm", 2);

                    land_prog->uniform("frag_d", frag_d[0], frag_d[1]);
                    land_prog->uniform("frag_k", frag_k[0], frag_k[1]);

                    ren.bind();
                    dat.idx()->bind();
                    vtx.bind();
                    {
                        pass();

                        // HACK.  Simple yet surprisingly effective.
/*
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glLineWidth(2.0f);
                        pass();
                        glLineWidth(1.0f);
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
*/
                    }
                    vtx.free();
                    dat.idx()->free();
                    ren.free();
                }
                land_prog->free();
            }
            glPopClientAttrib();
        }

        // Draw the atmosphere.

        if (::conf->get_i("atmo"))
        {
            glEnable(GL_DEPTH_CLAMP_NV);
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glDisable(GL_LIGHTING);

                atmo_prog->bind();
                {
                    atmo_pool->draw_init();
                    atmo_pool->draw(1, true, false);
                    atmo_pool->draw_fini();
                }
                atmo_prog->free();
            }
            glDisable(GL_DEPTH_CLAMP_NV);
        }
    }
    glPopAttrib();
}

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

//-----------------------------------------------------------------------------

bool uni::sphcmp(const sphere *A, const sphere *B)
{
    return (A->distance() < B->distance());
}

//-----------------------------------------------------------------------------
