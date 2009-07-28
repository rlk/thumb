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

    uint32_t up;
    uint32_t ch[4];
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
    name(name),
    buff(0),
    head(0),
    page(0),
    vert(0),
    bound(0),
    vbo(0),
    s(100),
    h(10)
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

    // Initialize the GL state.

    init();
}

ogl::terrain::~terrain()
{
    // Finalize the GL state.

    fini();

    // Release the data.

    if (!data.empty()) ::data->free(data);
}

//-----------------------------------------------------------------------------

static GLushort rdn2(GLushort i) { return ((i    ) / 2) * 2; }
static GLushort rup2(GLushort i) { return ((i + 1) / 2) * 2; }

static bool dif(GLushort i0, GLushort i1, GLushort i2)
{
    if (i0 == i1) return false;
    if (i1 == i2) return false;
    if (i2 == i0) return false;

    return true;
}

// Calculate an index buffer for a geo-mipmap page.  Booleans n, s, e, w
// indicate that page edge in the corresponding direction has low resolution.

GLsizei ogl::terrain::calc_index(GLuint o, bool bn, bool bs,
                                           bool be, bool bw)
{
    // Initialize the index buffer.

    const GLushort n = head->n;

    std::vector<GLushort> index;

    for     (int i = 0; i < int(head->n) - 1; ++i)
        for (int j = 0; j < int(head->n) - 1; ++j)
        {
            GLushort r0 = i,     c0 = j;
            GLushort r1 = i,     c1 = j + 1;
            GLushort r2 = i + 1, c2 = j;
            GLushort r3 = i + 1, c3 = j + 1;

            if ((i * 2) / (n - 1) == (j * 2) / (n - 1))
            {
                if (bs && i == 0)     { c0 = rdn2(c0); c1 = rdn2(c1); }
                if (bn && i == n - 2) { c2 = rup2(c2); c3 = rup2(c3); }
                if (bw && j == 0)     { r0 = rdn2(r0); r2 = rdn2(r2); }
                if (be && j == n - 2) { r1 = rup2(r1); r3 = rup2(r3); }

                if (dif(r0 * n + c0, r2 * n + c2, r3 * n + c3))
                {
                    index.push_back(r0 * n + c0);
                    index.push_back(r3 * n + c3);
                    index.push_back(r2 * n + c2);
                }

                if (dif(r0 * n + c0, r1 * n + c1, r3 * n + c3))
                {
                    index.push_back(r3 * n + c3);
                    index.push_back(r0 * n + c0);
                    index.push_back(r1 * n + c1);
                }
            }
            else
            {
                if (bs && i == 0)     { c0 = rup2(c0); c1 = rup2(c1); }
                if (bn && i == n - 2) { c2 = rdn2(c2); c3 = rdn2(c3); }
                if (bw && j == 0)     { r0 = rup2(r0); r2 = rup2(r2); }
                if (be && j == n - 2) { r1 = rdn2(r1); r3 = rdn2(r3); }

                if (dif(r0 * n + c0, r1 * n + c1, r2 * n + c2))
                {
                    index.push_back(r1 * n + c1);
                    index.push_back(r2 * n + c2);
                    index.push_back(r0 * n + c0);
                }

                if (dif(r1 * n + c1, r2 * n + c2, r3 * n + c3))
                {
                    index.push_back(r2 * n + c2);
                    index.push_back(r1 * n + c1);
                    index.push_back(r3 * n + c3);
                }
            }
        }

    // Copy the indices to an element buffer object.

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, o);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
                     index.size() * sizeof (GLushort),
                    &index.front(), GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    return GLsizei(index.size());
}

// Recursively compute a view-space axis-aligned bounding box for page k.
// The values [x0,x1] [y0,y1] give the planar bounds of page k implicitly
// while the z bounds are explicit within the file.  

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

    if (page[k].ch[0]) calc_bound(x0, y0, xm, ym, page[k].ch[0]);
    if (page[k].ch[1]) calc_bound(xm, y0, x1, ym, page[k].ch[1]);
    if (page[k].ch[2]) calc_bound(x0, ym, xm, y1, page[k].ch[2]);
    if (page[k].ch[3]) calc_bound(xm, ym, x1, y1, page[k].ch[3]);
}

//-----------------------------------------------------------------------------

ogl::range ogl::terrain::view(const double *V, int n) const
{
    // This can be minimized, though to do so may result in range popping.
    
    return bound[0].get_range(V);
}

//-----------------------------------------------------------------------------

// Determine whether the resolution and distance to page k indicate that
// it should be drawn (regardless of its visibility).

bool ogl::terrain::page_test(int k, const double *p) const
{
    double d = bound[k].get_distance(p);
    double e =  page[k].err * h / 65536;

    if (d > 0.0)
        return (atan2(e, d) < 0.01);
    else
        return !(page[k].ch[0] && page[k].ch[1] &&
                 page[k].ch[2] && page[k].ch[3]);
}

// TODO: This does not properly enforce the constraint that adjacent pages
// be within one level-of-detail of one another.

void ogl::terrain::page_draw(int k, int kn, int ks, int ke, int kw,
                             const double *p, const double *V, int n) const
{
    if (bound[k].test(V, n))
    {
        if (page_test(k, p))
        {
            GLsizei vs = sizeof (vert_s);

            const int bn = ((kn == 0) || !page_test(page[kn].up, p)) ? 0 : 1;
            const int bs = ((ks == 0) || !page_test(page[ks].up, p)) ? 0 : 2;
            const int be = ((ke == 0) || !page_test(page[ke].up, p)) ? 0 : 4;
            const int bw = ((kw == 0) || !page_test(page[kw].up, p)) ? 0 : 8;

            const int b = bn + bs + be + bw;

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo[b]);

            glVertexPointer(3, GL_SHORT,
                            vs, (GLvoid *) (vs * head->n * head->n * k));

            glDrawElements(GL_TRIANGLES, ibc[b],
                           GL_UNSIGNED_SHORT, 0);
        }
        else
        {
            const int k0 = page[k].ch[0];
            const int k1 = page[k].ch[1];
            const int k2 = page[k].ch[2];
            const int k3 = page[k].ch[3];

            if (k0) page_draw(k0, k2, ks ? page[ks].ch[2] : 0,
                                  k1, kw ? page[kw].ch[1] : 0, p, V, n);
            if (k1) page_draw(k1, k3, ks ? page[ks].ch[3] : 0,
                                  ke ? page[ke].ch[0] : 0, k0, p, V, n);
            if (k2) page_draw(k2, kn ? page[kn].ch[0] : 0, k0,
                                  k3, kw ? page[kw].ch[3] : 0, p, V, n);
            if (k3) page_draw(k3, kn ? page[kn].ch[1] : 0, k1,
                                  ke ? page[ke].ch[2] : 0, k2, p, V, n);
        }
    }
}

//-----------------------------------------------------------------------------

void ogl::terrain::draw(const double *p,
                        const double *V, int n) const
{
    if (page && head->c > 0)
    {
        glUseProgramObjectARB(0);
        glPointSize(3.0);
        glLineWidth(1.0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnableClientState(GL_VERTEX_ARRAY);
        {
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

            glColor4f(1.0f, 1.0f, 0.0f, 0.25f);
            glPushMatrix();
            {
                glMultMatrixd(M);
                page_draw(0, 0, 0, 0, 0, p, V, n);
            }
            glPopMatrix();
/*
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glColor4f(1.0f, 1.0f, 0.0f, 0.25f);
            glPushMatrix();
            {
                glMultMatrixd(M);
                page_draw(0, 0, 0, 0, 0, p, V, n);
            }
            glPopMatrix();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
*/

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

//-----------------------------------------------------------------------------

void ogl::terrain::init()
{
    if (head->c && vert)
    {
        // Initialize the vertex buffer object.

        glGenBuffersARB(1, &vbo);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        head->n * head->n * head->c * sizeof (vert_s), vert,
                        GL_STATIC_DRAW_ARB);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        // Initialize the index buffer objects.

        glGenBuffersARB(16, ibo);

        ibc[ 0] = calc_index(ibo[ 0], false, false, false, false);
        ibc[ 1] = calc_index(ibo[ 1],  true, false, false, false);
        ibc[ 2] = calc_index(ibo[ 2], false,  true, false, false);
        ibc[ 3] = calc_index(ibo[ 3],  true,  true, false, false);
        ibc[ 4] = calc_index(ibo[ 4], false, false,  true, false);
        ibc[ 5] = calc_index(ibo[ 5],  true, false,  true, false);
        ibc[ 6] = calc_index(ibo[ 6], false,  true,  true, false);
        ibc[ 7] = calc_index(ibo[ 7],  true,  true,  true, false);
        ibc[ 8] = calc_index(ibo[ 8], false, false, false,  true);
        ibc[ 9] = calc_index(ibo[ 9],  true, false, false,  true);
        ibc[10] = calc_index(ibo[10], false,  true, false,  true);
        ibc[11] = calc_index(ibo[11],  true,  true, false,  true);
        ibc[12] = calc_index(ibo[12], false, false,  true,  true);
        ibc[13] = calc_index(ibo[13],  true, false,  true,  true);
        ibc[14] = calc_index(ibo[14], false,  true,  true,  true);
        ibc[15] = calc_index(ibo[15],  true,  true,  true,  true);
    }
}

void ogl::terrain::fini()
{
    glDeleteBuffers(16, ibo);
    glDeleteBuffers(1, &vbo);
    vbo = 0;
}

//-----------------------------------------------------------------------------
