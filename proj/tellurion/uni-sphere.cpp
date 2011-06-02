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

#include "ogl-pool.hpp"
#include "ogl-opengl.hpp"
#include "ogl-program.hpp"
#include "etc-math.hpp"
#include "uni-sphere.hpp"
#include "app-glob.hpp"
#include "app-host.hpp"
#include "app-prog.hpp"
#include "app-user.hpp"
#include "app-default.hpp"

//=============================================================================

uni::sphere::sphere(uni::geodat& dat,
                    uni::georen& ren,
                    uni::geomap_l& color,
                    uni::geomap_l& normal,
                    uni::geomap_l& height,
                    uni::geocsh_l& caches,
                    double r0,
                    double r1, GLsizei lines, bool atmo_p, bool over_p) :

    count(0),

    r0(r0),
    r1(r1),
    a0(r0 * 1.000),
    a1(a0 * 1.025),

    atmosphere(atmo_p),
    visible(false),
    dist(0),

    lines(lines),

    dat(dat),
    tex(dat.depth(), lines),
    nrm(dat.depth(), lines),
    pos(dat.depth(), lines),
    acc(dat.depth(), lines),
#ifdef CONF_CALCEXT
    ext(dat.depth(), lines),
#endif
    vtx(dat.depth(), lines),
    ren(ren),

    color (color),
    normal(normal),
    height(height),
    caches(caches),

    draw_atmo_in (glob->load_program("SkyFromAtmosphere.xml")),
    draw_atmo_out(glob->load_program("SkyFromSpace.xml")),
    draw_land_in (glob->load_program("GroundFromAtmosphere.xml")),
    draw_land_out(glob->load_program("GroundFromSpace.xml")),

    draw_land(glob->load_program("final-land.xml")),
    draw_diff(glob->load_program("final-diff.xml")),
    draw_norm(glob->load_program("final-norm.xml")),
    draw_texc(glob->load_program("final-texc.xml")),
    draw_mono(glob->load_program("final-mono.xml")),
    draw_dtex(glob->load_program("final-dtex.xml")),

    atmo_pool(0),
    atmo_node(0),
    atmo_unit(0),

    over(0)
{
    S = new spatch[lines];

    // Set default configuration.
    
    memset(p, 0, 3 * sizeof (double));

    n[0]  = 0;
    n[1]  = 1;
    n[2]  = 0;
    
    d0   = 1.0;
    d1   = 2.0;

    // Initialize atmosphere rendering.

    atmo_pool = glob->new_pool();
    atmo_node = new ogl::node();
    atmo_unit = new ogl::unit("solid/inverted_sphere.obj");

    atmo_pool->add_node(atmo_node);
    atmo_node->add_unit(atmo_unit);

    double M[16];
    double I[16];

    load_scl_mat(M, a1, a1, a1);
    load_scl_inv(I, a1, a1, a1);

    atmo_unit->transform(M, I);

    // Initialize overlay rendering.

    if (over_p)
        over = new overlay(r0);
}

uni::sphere::~sphere()
{
    while (!frustums.empty())
    {
        delete frustums.back();
        frustums.pop_back();
    }

    if (over) delete over;

    glob->free_program(draw_dtex);
    glob->free_program(draw_mono);
    glob->free_program(draw_texc);
    glob->free_program(draw_norm);
    glob->free_program(draw_diff);
    glob->free_program(draw_land);

    glob->free_program(draw_land_out);
    glob->free_program(draw_land_in);
    glob->free_program(draw_atmo_out);
    glob->free_program(draw_atmo_in);

    if (atmo_pool) glob->free_pool(atmo_pool);

    delete [] S;
}

void uni::sphere::move(double px, double py, double pz,
                       double nx, double ny, double nz)
{
    p[0] = px;
    p[1] = py;
    p[2] = pz;

    n[0] = nx;
    n[1] = ny;
    n[2] = nz;

    normalize(n);
}
/*
void uni::sphere::turn(double da, double dt)
{
    angle += da;
    tilt  += dt;

    norm();
}

void uni::sphere::norm()
{
    double N[3] = { 0, 1, 0 };
    double M[16];

    double tilt = 0.0;

    load_rot_mat(M, 1, 0, 0, tilt);

    mult_mat_vec3(n, M, N);
    normalize(n);
}
*/
//-----------------------------------------------------------------------------

void uni::sphere::atmo_prep(const ogl::program *P) const
{
    P->bind();
    {
        double r = sqrt(DOT3(vp, vp));

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

void uni::sphere::transform(int frusc, const app::frustum *const *frusv)
{
    double angle = ::user->get_t() * 360.0 / (60.0 * 60.0 * 24.0);
    double tilt  = 0.0;

    // Compute the planetary tilt transformation.

    load_rot_mat(O, 1, 0, 0, tilt);     // Planet tilt
    Rmul_rot_mat(O, 0, 1, 0, angle);    // Planet rotation

    // Compose the object-to-camera transformation.

    load_mat(M, ::user->get_I());

    Rmul_xlt_mat(M, p[0], p[1], p[2]);  // Planet position
    Rmul_rot_mat(M, 1, 0, 0, tilt);     // Planet tilt
    Rmul_rot_mat(M, 0, 1, 0, angle);    // Planet rotation

    // Compose the camera-to-object transform.

    load_mat(I, ::user->get_M());

    Lmul_xlt_inv(I, p[0], p[1], p[2]);  // Planet position
    Lmul_rot_inv(I, 1, 0, 0, tilt);     // Planet tilt
    Lmul_rot_inv(I, 0, 1, 0, angle);    // Planet rotation

    // HACK: inefficient

    while (!frustums.empty())
    {
        delete frustums.back();
        frustums.pop_back();
    }

    // Apply the transform to the frusta.

    for (int frusi = 0; frusi < frusc; ++frusi)
    {
        app::frustum *frusp = new app::frustum(*(frusv[frusi]));

        frusp->set_transform(I);
        frusp->set_horizon(r0);

        frustums.push_back(frusp);
    }

    // Apply the transform to the atmosphere.

    if (atmo_node) atmo_node->transform(M);
    if (over)           over->transform(M, I);
}

//-----------------------------------------------------------------------------

void uni::sphere::view(int frusc, const app::frustum *const *frusv)
{
    // TODO: test for sphere visibility

    if ((visible = true))
    {
        // Compose the local transform, and cache the local view frustums.

        transform(frusc, frusv);

        // Cache the view position.

        if (!frustums.empty())
        {
            const double *p = frustums[0]->get_view_pos();

            vp[0] = p[0];
            vp[1] = p[1];
            vp[2] = p[2];

            dist = sqrt(DOT3(vp, vp));
        }
        else
        {
            vp[0] = 0.0;
            vp[1] = 0.0;
            vp[2] = 0.0;

            dist = 0;
        }

        // Prep the atmosphere model.

        if (atmosphere)
        {
            atmo_pool->prep();
            atmo_pool->view(1, 0, 0);
        }
        if (over) over->prep();
    }
}

bool uni::sphere::test(const double *n0,
                       const double *n1,
                       const double *n2) const
{
    // HACK HACK

    double N[3];

    N[0] = n0[0] + n1[0] + n2[0];
    N[1] = n0[1] + n1[1] + n2[1];
    N[2] = n0[2] + n1[2] + n2[2];

    normalize(N);

    double d =      DOT3(n0, N);
    d = std::min(d, DOT3(n1, N));
    d = std::min(d, DOT3(n2, N));

    double a = acos(d);

    // Return true if any part of the given shell falls within any frustum.

    std::vector<app::frustum *>::const_iterator i;

    for (i = frustums.begin(); i != frustums.end(); ++i)
        if ((*i)->test_cap(N, a, r0, r1) >= 0)
            return true;
/*
        if ((*i)->test_shell(n0, n1, n2, r0, r1) >= 0)
            return true;
*/
    return false;
}

void uni::sphere::step(int serial)
{
    count = 0;

    if (visible)
    {
        // Refine the icosahedron to fit the current view.

        int ii[20];

        for (int k = 0; k < 20; ++k)
        {
            const int    *i  = dat.ico()->point_i(k);

            const double *n0 = dat.ico()->point_v(i[0]);
            const double *n1 = dat.ico()->point_v(i[1]);
            const double *n2 = dat.ico()->point_v(i[2]);

            if (test(n0, n1, n2))
                ii[k] = count++;
            else
                ii[k] = -1;
        }

        for (int k = 0; k < 20; ++k)
        {
            const int    *i  = dat.ico()->point_i(k);
            const int    *j  = dat.ico()->patch_i(k);

            const double *n0 = dat.ico()->point_v(i[0]);
            const double *n1 = dat.ico()->point_v(i[1]);
            const double *n2 = dat.ico()->point_v(i[2]);

            if (ii[k] >= 0)
                S[ii[k]].init(n0, dat.ico()->patch_c(k, 0), ii[j[0]],
                              n1, dat.ico()->patch_c(k, 1), ii[j[1]],
                              n2, dat.ico()->patch_c(k, 2), ii[j[2]],
                              vp, r0, 0);
        }

        while (0 < count && count < lines)
        {
            int i = 0;

            for (int j = 0; j < count; ++j)
                if (S[j].more(S + i))
                    i = j;

            if (S[i].k < 0 || !S[i].subd(S, i, count, lines, r0, r1, frustums, vp))
                break;
        }

        // Set up geometry generation.

        if (count)
        {
            double Z1[3], Z0[3] = { 0.0, 0.0, 0.0 };

            mult_mat_vec3(Z1, M, Z0);

            // Seed the normal and position generators.

            nrm.init();
            pos.init(r0, Z1);
            tex.init();
            {
                for (int k = 0; k < count; ++k)
                    S[k].seed(k, nrm, pos, tex, r0, M, I);
            }
            tex.fini(count);
            nrm.fini(count);
            pos.fini(count);

            // Store the depth range. (HACK out extrema reduction)

            d0 = pos.min_d();
            d1 = pos.max_d();
        }

        // Update the map page states.

        geomap_i m;
    
        int                        frusc =  frustums.size();
        const app::frustum *const *frusv = &frustums.front();

        for (m =  color.begin(); m !=  color.end(); ++m)
            (*m)->view(frusc, frusv, r0, r1, serial);
        for (m = normal.begin(); m != normal.end(); ++m)
            (*m)->view(frusc, frusv, r0, r1, serial);
        for (m = height.begin(); m != height.end(); ++m)
            (*m)->view(frusc, frusv, r0, r1, serial);
    }
}

void uni::sphere::prep()
{
    if (count)
    {
        // Compute the eye-to-object normal matrix transpose.

        dat.lut()->bind(GL_TEXTURE1);
        dat.rad()->bind(GL_TEXTURE3);
        {
            // Generate normals.

            nrm.proc(count);

            nrm.bind(GL_TEXTURE2);
            {
                // Generate texture coordinates.

                tex.uniform("M", M, true);
                tex.proc(count);

                // Generate positions.

                pos.proc(count);
            }
            nrm.free(GL_TEXTURE2);
        }
        dat.rad()->free(GL_TEXTURE3);
        dat.lut()->free(GL_TEXTURE1);

        // Bind all generated attributes for use in terrain accumulation.

        pos.bind(GL_TEXTURE4);
        nrm.bind(GL_TEXTURE5);
        tex.bind(GL_TEXTURE6);
        {
            double Z1[3], Z0[3] = { 0.0, 0.0, 0.0 };

            mult_mat_vec3(Z1, M, Z0);

            // Accumulate terrain maps.

            acc.init(count, r0, Z1);
            acc.bind_proc(); // overkill
            {
                glEnable(GL_BLEND);
                {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                    for (geomap_i m = height.begin(); m != height.end(); ++m)
                        (*m)->draw(r0, Z1);
                }
                glDisable(GL_BLEND);
            }
            acc.free_proc(); // overkill
            acc.swap();
        }
        tex.free(GL_TEXTURE6);
        nrm.free(GL_TEXTURE5);
        pos.free(GL_TEXTURE4);

        // Find the extrema of the accumulated positions.

#ifdef CONF_CALCEXT
        acc.bind(GL_TEXTURE1);
        {
            ext.proc(count);
        }
        acc.free(GL_TEXTURE1);
#endif

        // Copy the generated coordinates to the vertex buffer.

        tex.bind_frame();
        vtx.read_c(count);
        tex.free_frame();

        // Copy the generated normals to the vertex buffer.

        nrm.bind_frame();
        vtx.read_n(count);
        nrm.free_frame();

        // Copy the generated positions to the vertex buffer.

        if (height.empty())
        {
            pos.bind_frame();
            vtx.read_v(count);
            pos.free_frame();
        }
        else
        {
            acc.bind_frame();
            vtx.read_v(count);
            acc.free_frame();
        }

#ifdef CONF_CALCEXT
        const GLfloat *E = ext.rmap();

        d0 = sqrt(E[0]);
        d1 = sqrt(E[1]);
        ext.umap();
#endif
    }
}

void uni::sphere::pass()
{
    GLsizei vc = dat.vtx_len();
    GLsizei tc = dat.idx_len();

    for (int k = 0; k < count; ++k)
        S[k].draw(S, k, lines, vc, tc);
}

void uni::sphere::draw(int i)
{
    bool in = (sqrt(DOT3(vp, vp)) < a1);

    // Translate application toggles into shader selections.

    const ogl::program *atmo_prog = 0;
    const ogl::program *land_prog = 0;
/*
    if      (::prog->get_option(0)) land_prog = draw_diff;
    else if (::prog->get_option(1)) land_prog = draw_norm;
    else if (::prog->get_option(2)) land_prog = draw_texc;
    else if (::prog->get_option(3)) land_prog = draw_mono;
    else if (::prog->get_option(4)) land_prog = draw_dtex;
*/
    if (atmosphere)
    {
        atmo_prog = in ? draw_atmo_in : draw_atmo_out;
        land_prog = in ? draw_land_in : draw_land_out;

        atmo_prep(atmo_prog);
        atmo_prep(land_prog);
    }
    else
    {
        land_prog = draw_land;
    }

    ogl::curr_texture(GL_TEXTURE0);

    // TODO: calc_projection can move to whereever the bounds are first known.

    frustums[i]->set_distances(d0 / 2, d1);
    frustums[i]->draw();

    glLoadIdentity();

    // Position the sun.

    GLfloat L[4];
    double  t[4];
/* HACK
    double  sun[4] = { 0.0, 0.0, 149000000000.0 };

    mult_xps_vec3(t, O, sun);
*/
    t[0] = vp[0];
    t[1] = vp[1];
    t[2] = vp[2];

    normalize(t);

    L[0] = GLfloat(t[0]);
    L[1] = GLfloat(t[1]);
    L[2] = GLfloat(t[2]);
    L[3] = 0;

    glLightfv(GL_LIGHT0, GL_POSITION, L);

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);
    {
        if (1 || count)
        {
            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            glEnable(GL_TEXTURE_2D);
//          glEnable(GL_NORMALIZE);

            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            {
                double T[16];

                load_xps(T, M);
                ogl::xfrm_texture(GL_TEXTURE0, T);

                glEnableClientState(GL_VERTEX_ARRAY);
                glEnableClientState(GL_NORMAL_ARRAY);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);

                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                // Draw the texture coordinates.

                ren.cyl()->init();
                ren.cyl()->bind();
                dat.idx()->bind();
                vtx.bind();
                {
                    pass();
                }
                vtx.free();
                dat.idx()->free();
                ren.cyl()->free();

                // Accumulate the textures.

                glDisable(GL_DEPTH_TEST);

                ren.dif()->init(1.0f, 1.0f, 1.0f);

//              if (!::prog->get_option(2))
                {
                    ren.dif()->bind();
                    {
                        for (geomap_i m =  color.begin(); m !=  color.end(); ++m)
                            (*m)->draw(0, 0);
                    }
                    ren.dif()->free();
                }

                ren.nrm()->init();
                ren.nrm()->bind();
                {
                    for (geomap_i m = normal.begin(); m != normal.end(); ++m)
                        (*m)->draw(0, 0);
                }
                ren.nrm()->free();

                // Draw the illuminated geometry.

                glClear(GL_DEPTH_BUFFER_BIT);
                glEnable(GL_DEPTH_TEST);
                {
                    land_prog->bind();
                    {
                        ren.bind();
                        dat.idx()->bind();
                        vtx.bind();
                        {
                            pass();
/*
                            if (!::prog->get_option(11))
                            {
                                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                pass();
                                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                            }
*/
                        }
                        vtx.free();
                        dat.idx()->free();
                        ren.free();
                    }
                    land_prog->free();
                }

                // Draw patch bounds as requested.
/*
                if (::prog->get_option(10))
                {
                    glPushMatrix();
                    {
                        glLoadMatrixd(M);

                        glPushAttrib(GL_ENABLE_BIT);
                        {
                            glDisable(GL_TEXTURE_2D);
                            glDisable(GL_LIGHTING);

                            glEnable(GL_BLEND);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                            for (int k = 0; k < count; ++k)
                                S[k].wire(r0, r1);
                        }
                        glPopAttrib();
                    }
                    glPopMatrix();
                }
*/
                // Draw wireframe as requested.
/*
                if (::prog->get_option(11))
                {
                    ren.bind();
                    dat.idx()->bind();
                    vtx.bind();
                    {
                        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT);
                        {
                            glEnable(GL_BLEND);
                            glEnable(GL_LINE_SMOOTH);
                            glEnable(GL_POLYGON_OFFSET_LINE);
                            glDisable(GL_CULL_FACE);
                            glDisable(GL_LIGHTING);

                            glLineWidth(0.5);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                            glPolygonOffset(-0.5, -0.5);

                            glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
                            pass();
                            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                        }
                        glPopAttrib();
                    }
                    vtx.free();
                    dat.idx()->free();
                    ren.free();
                }
*/
            }
            glPopClientAttrib();
        }

        // Draw the overlay models.

        if (over)
            over->draw_models();
        
        // Draw the atmosphere.

        if (atmo_prog)
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

        // Test draw the color geomap.

        glUseProgramObjectARB(0);

//      if (::prog->get_option(7)) (*(  caches.begin()))->draw();
//      if (::prog->get_option(6)) (*(++caches.begin()))->draw();

        (*(caches.begin()))->draw();

        // Test draw the GPGPU buffers.
/*
        glPushAttrib(GL_ENABLE_BIT);
        {
            int w = ::host->get_buffer_w();
            int h = ::host->get_buffer_h();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_DEPTH_TEST);

            if (::prog->get_option(5)) acc.draw(w, h);
            if (::prog->get_option(8)) nrm.draw(w, h);
            if (::prog->get_option(9)) tex.draw(w, h);
        }
        glPopAttrib();
*/
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

bool uni::sphcmp(const sphere *A, const sphere *B)
{
    return (A->distance() < B->distance());
}

//-----------------------------------------------------------------------------
