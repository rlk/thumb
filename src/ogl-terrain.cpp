//  Copyright (C) 2009 Robert Kooima
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

#include "matrix.hpp"
#include "app-data.hpp"
#include "app-file.hpp"
#include "ogl-terrain.hpp"

//-----------------------------------------------------------------------------

struct ogl::head_s
{
    uint32_t c;
    uint16_t n;
    uint16_t m;
};

struct ogl::page_s
{
    int16_t min;
    int16_t max;
    int16_t err;
    int16_t pad;

    uint32_t sub[4];
};

struct ogl::vert_s
{
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t u;
};

//-----------------------------------------------------------------------------

ogl::terrain::terrain(std::string name) :
    name(name), buff(0), head(0), page(0), vert(0), bound(0), s(100), h(10)
{
    app::file file("terrain/" + name);

    // Parse the configuration.

    if (app::node p = file.get_root().find("terrain"))
    {
        data = p.get_s("file");
        s    = p.get_f("size",  100.0);
        h    = p.get_f("height", 10.0);

        if (!data.empty())
        {
            // Load the data.

            if ((buff = (const uint8_t *) ::data->load("terrain/" + data)))
            {
                head = (const head_s *) (buff);
                page = (const page_s *) (buff + sizeof (head_s));
                vert = (const vert_s *) (buff + sizeof (head_s)
                                              + sizeof (page_s) * head->c);
            }
        }
    }

    // Initialize the transformation.

    double xs = head ? s / head->m : 1.0;
    double zs =        h / 65536.0;

    load_rot_mat(M, 1, 0, 0, -90);
    Rmul_scl_mat(M, xs, xs, zs);

    load_rot_inv(I, 1, 0, 0, -90);
    Lmul_scl_inv(I, xs, xs, zs);

    // Intialize the view-space bounding volume cache.

    if (page && head->c)
    {
        const int m = (head->m - 1) / 2;

        bound.reserve(head->c);
        bound.resize (head->c);

        calc_bound(-m, -m, +m, +m, 0);
    }
}

ogl::terrain::~terrain()
{
    if (!data.empty()) ::data->free(data);
}

//-----------------------------------------------------------------------------

void ogl::terrain::calc_bound(int16_t x0, int16_t y0,
                              int16_t x1, int16_t y1, uint32_t k)
{
    double p0[3], v0[3];
    double p1[3], v1[3];

    // Copy the object-space extrema.

    p0[0] = x0;
    p0[1] = y0;
    p0[2] = page[k].min;

    p1[0] = x1;
    p1[1] = y1;
    p1[2] = page[k].max;

    // Transform the extrema into view space.

    mult_mat_vec3(v0, M, p0);
    mult_mat_vec3(v1, M, p1);

    // Store the extrema in an AABB object.

    bound[k].merge(v0[0], v0[1], v0[2]);
    bound[k].merge(v1[0], v1[1], v1[2]);

    // Recursively evaluate any children.

    const int xm = (x0 + x1) / 2;
    const int ym = (y0 + y1) / 2;

    if (page[k].sub[0]) calc_bound(x0, y0, xm, ym, page[k].sub[0]);
    if (page[k].sub[1]) calc_bound(xm, y0, x1, ym, page[k].sub[1]);
    if (page[k].sub[2]) calc_bound(x0, ym, xm, y1, page[k].sub[2]);
    if (page[k].sub[3]) calc_bound(xm, ym, x1, y1, page[k].sub[3]);
}

//-----------------------------------------------------------------------------

ogl::range ogl::terrain::view(const double *V, int n) const
{
    // This can be minimized, though to do so may result in range popping.
    
    return bound[0].get_range(V);
}

//-----------------------------------------------------------------------------
#if 0
void ogl::terrain::traverse(int k, int kn, int ks, int ke, int kw) const
{
    if (page[k].sub[0])
        traverse(page[k].sub[0], k2, ks ? page[ks].sub[2] : 0,
                                 k1, kw ? page[kw].sub[1] : 0);
    if (page[k].sub[1])
        traverse(page[k].sub[1], k3, ks ? page[ks].sub[3] : 0,
                                 ke ? page[ke].sub[0] : 0, k0);
    if (page[k].sub[2])
        traverse(page[k].sub[2], kn ? page[kn].sub[0] : 0, k0,
                                 k3, kw ? page[kw].sub[3] : 0);
    if (page[k].sub[3])
        traverse(page[k].sub[3], kn ? page[kn].sub[1] : 0, k1,
                                 ke ? page[ke].sub[2] : 0, k2);
}
#endif
//-----------------------------------------------------------------------------

// Determine whether the resolution and distance to page k indicate that
// it should be drawn (regardless of its visibility).

bool ogl::terrain::page_test(int k, const double *p) const
{
    double d = bound[k].get_distance(p);
    double e =  page[k].err * h / 65536;

    return (d > 0.0) ? (atan2(e, d) < 0.01) : true;
}

void ogl::terrain::page_draw(int k, int kn, int ks, int ke, int kw,
                             const double *p, const double *V, int n) const
{
    if (bound[k].test(V, n))
    {
        if (page_test(k, p))
        {
            const bool bn = kn ? page_test(kn, p) : false;
            const bool bs = ks ? page_test(ks, p) : false;
            const bool be = ke ? page_test(ke, p) : false;
            const bool bw = kw ? page_test(kw, p) : false;

            bound[k].draw(bn, bs, be, bw);
        }
        else 
        {
            const int k0 = page[k].sub[0];
            const int k1 = page[k].sub[1];
            const int k2 = page[k].sub[2];
            const int k3 = page[k].sub[3];

            if (k0) page_draw(k0, k2, ks ? page[ks].sub[2] : 0,
                                  k1, kw ? page[kw].sub[1] : 0, p, V, n);
            if (k1) page_draw(k1, k3, ks ? page[ks].sub[3] : 0,
                                  ke ? page[ke].sub[0] : 0, k0, p, V, n);
            if (k2) page_draw(k2, kn ? page[kn].sub[0] : 0, k0,
                                  k3, kw ? page[kw].sub[3] : 0, p, V, n);
            if (k3) page_draw(k3, kn ? page[kn].sub[1] : 0, k1,
                                  ke ? page[ke].sub[2] : 0, k2, p, V, n);
        }
    }
}

//-----------------------------------------------------------------------------
/*
double ogl::terrain::page_lod(int k, const double *p) const
{
    double d = bound[k].get_distance(p);
    double e =  page[k].err * h / 65536;

    if (d > 0)
        return atan2(e, d);
    else
        return M_PI * 2.0;
}

void ogl::terrain::page_draw(int k, const double *p,
                                    const double *V, int n) const
{
    if (bound[k].test(V,n))
    {
        const int k0 = page[k].sub[0];
        const int k1 = page[k].sub[1];
        const int k2 = page[k].sub[2];
        const int k3 = page[k].sub[3];

        if (!k0 || !k1 || !k2 || !k3 || page_lod(k, p) < 0.01)
        {
            glBegin(GL_POINTS);
            {
                const vert_s *v = vert + head->n * head->n * k;

                for (int i = 0; i < head->n * head->n; ++i)
                    glVertex3sv((const GLshort *) (v + i));
            }
            glEnd();
        }
        else
        {
            if (k0) page_draw(k0, p, V, n);
            if (k1) page_draw(k1, p, V, n);
            if (k2) page_draw(k2, p, V, n);
            if (k3) page_draw(k3, p, V, n);
        }
    }
}
*/
void ogl::terrain::draw(const double *p,
                        const double *V, int n) const
{
    if (page && head->c > 0)
    {
        glUseProgramObjectARB(0);
        glPointSize(3.0);

        glColor3f(1.0f, 1.0f, 0.0);

        glPushMatrix();
        {
//          glMultMatrixd(M);

            page_draw(0, 0, 0, 0, 0, p, V, n);
        }
        glPopMatrix();

        glColor3f(1.0f, 0.5f, 0.0);

//      bound[0].draw();
    }
}

//-----------------------------------------------------------------------------

void ogl::terrain::init()
{
}

void ogl::terrain::fini()
{
}

//-----------------------------------------------------------------------------
