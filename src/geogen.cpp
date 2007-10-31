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
#include <cmath>
#include <queue>
#include <limits>

#include "opengl.hpp"
#include "geogen.hpp"
#include "matrix.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

// The number of vertices in a D-fold recursively subdivided triangle.

static GLsizei vtx_count(GLsizei d)
{
    GLsizei n = 1 << d;

    return (n + 1) * (n + 2) / 2;
}

// The number of triangles in a D-fold recursively subdivided triangle.

static GLsizei tri_count(GLsizei d)
{
    return d ? 4 * tri_count(d - 1) : 1;
}

//-----------------------------------------------------------------------------

// Patch points: point_i[p][i] is the i-th point of patch p.

int uni::geoico::_point_i[20][3] = {
    {  0,  1,  2 }, {  0,  2,  3 }, {  0,  3,  4 }, {  0,  4,  5 },
    {  0,  5,  1 }, {  1,  7,  2 }, {  2,  8,  3 }, {  3,  9,  4 },
    {  4, 10,  5 }, {  5,  6,  1 }, {  1,  6,  7 }, {  2,  7,  8 },
    {  3,  8,  9 }, {  4,  9, 10 }, {  5, 10,  6 }, {  6, 11,  7 },
    {  7, 11,  8 }, {  8, 11,  9 }, {  9, 11, 10 }, { 10, 11,  6 },
};

// Patch adjacency: patch_i[p][i] is the i-th neighbor of patch p.

int uni::geoico::_patch_i[20][3] = {
    {  4,  5,  1 }, {  0,  6,  2 }, {  1,  7,  3 }, {  2,  8,  4 },
    {  3,  9,  0 }, { 10, 11,  0 }, { 11, 12,  1 }, { 12, 13,  2 },
    { 13, 14,  3 }, { 14, 10,  4 }, {  9, 15,  5 }, {  5, 16,  6 },
    {  6, 17,  7 }, {  7, 18,  8 }, {  8, 19,  9 }, { 19, 16, 10 },
    { 15, 17, 11 }, { 16, 18, 12 }, { 17, 19, 13 }, { 18, 15, 14 },
};

// Patch context: p is the patch_j[p][i]-th neighbor of patch_i[p][i].

int uni::geoico::_patch_j[20][3] = {
    {  2,  2,  0 }, {  2,  2,  0 }, {  2,  2,  0 }, {  2,  2,  0 },
    {  2,  2,  0 }, {  2,  0,  1 }, {  2,  0,  1 }, {  2,  0,  1 },
    {  2,  0,  1 }, {  2,  0,  1 }, {  1,  2,  0 }, {  1,  2,  0 },
    {  1,  2,  0 }, {  1,  2,  0 }, {  1,  2,  0 }, {  1,  0,  1 },
    {  1,  0,  1 }, {  1,  0,  1 }, {  1,  0,  1 }, {  1,  0,  1 },
};

// Spherical mapping: patch_c[p][i] is the coordinate of point i of patch p.

#define P0   0.00000000000000
#define P1  63.43494882292201
#define P2 116.56505117707799
#define P3 180.00000000000000

double uni::geoico::_patch_c[20][3][2] = {
    {{  72.0, P3 }, {  36.0, P2 }, { 108.0, P2 }},
    {{ 144.0, P3 }, { 108.0, P2 }, { 180.0, P2 }},
    {{ 216.0, P3 }, { 180.0, P2 }, { 252.0, P2 }},
    {{ 288.0, P3 }, { 252.0, P2 }, { 324.0, P2 }},
    {{ 260.0, P3 }, { 324.0, P2 }, { 396.0, P2 }},

    {{  36.0, P2 }, {  72.0, P1 }, { 108.0, P2 }},
    {{ 108.0, P2 }, { 144.0, P1 }, { 180.0, P2 }},
    {{ 180.0, P2 }, { 216.0, P1 }, { 252.0, P2 }},
    {{ 252.0, P2 }, { 288.0, P1 }, { 324.0, P2 }},
    {{ 324.0, P2 }, { 360.0, P1 }, { 396.0, P2 }},

    {{  36.0, P2 }, {   0.0, P1 }, {  72.0, P1 }},
    {{ 108.0, P2 }, {  72.0, P1 }, { 144.0, P1 }},
    {{ 180.0, P2 }, { 144.0, P1 }, { 216.0, P1 }},
    {{ 252.0, P2 }, { 216.0, P1 }, { 288.0, P1 }},
    {{ 324.0, P2 }, { 288.0, P1 }, { 360.0, P1 }},

    {{   0.0, P1 }, {  36.0, P0 }, {  72.0, P1 }},
    {{  72.0, P1 }, { 108.0, P0 }, { 144.0, P1 }},
    {{ 144.0, P1 }, { 180.0, P0 }, { 216.0, P1 }},
    {{ 216.0, P1 }, { 252.0, P0 }, { 288.0, P1 }},
    {{ 288.0, P1 }, { 324.0, P0 }, { 360.0, P1 }},
};

// Point definitions: point_v[p] is the normal of point p.

double uni::geoico::_point_v[12][3];

uni::geoico::geoico()
{
    // Coordinates are in degrees (for convenience).  Convert to radians.

    for (int k = 0; k < 20; ++k)
        for (int i = 0; i < 3; ++i)
        {
            _patch_c[k][i][0] = RAD(_patch_c[k][i][0] - 180.0);
            _patch_c[k][i][1] = RAD(_patch_c[k][i][1] -  90.0);
        }

    // Compute the normal of each point.  (Some redundancy here.)

    for (int k = 0; k < 20; ++k)
        for (int i = 0; i < 3; ++i)
        {
            double *v = _point_v[_point_i[k][i]];
            double *c = _patch_c         [k][i];

            v[0] = sin(c[0]) * cos(c[1]);
            v[1] =             sin(c[1]);
            v[2] = cos(c[0]) * cos(c[1]);
        }
}

//-----------------------------------------------------------------------------

class triangle
{
    GLsizei r0, c0;
    GLsizei r1, c1;
    GLsizei r2, c2;
    GLsizei r3, c3;
    GLsizei r4, c4;
    GLsizei r5, c5;

    GLsizei d;

public:

    triangle(GLsizei r0, GLsizei c0,
             GLsizei r1, GLsizei c1,
             GLsizei r2, GLsizei c2, GLsizei d) :

        // Store the triangle corners.

        r0(r0), c0(c0),
        r1(r1), c1(c1),
        r2(r2), c2(c2),

        // Compute the edge midpoints.

        r3((r1 + r2) / 2), c3((c1 + c2) / 2),
        r4((r2 + r0) / 2), c4((c2 + c0) / 2),
        r5((r0 + r1) / 2), c5((c0 + c1) / 2),

        d(d) { }

    void proc(GLushort *p, GLushort n, GLushort &i, std::queue<triangle>& Q)
        {
            // Assign a unique index to each midpoint vertex.

            if (d > 0)
            {
                if (p[n * r3 + c3] == 0) p[n * r3 + c3] = i++;
                if (p[n * r4 + c4] == 0) p[n * r4 + c4] = i++;
                if (p[n * r5 + c5] == 0) p[n * r5 + c5] = i++;
            }

            // Subdivide as necessary.

            if (d > 1)
            {
                Q.push(triangle(r0, c0, r5, c5, r4, c4, d - 1));
                Q.push(triangle(r5, c5, r1, c1, r3, c3, d - 1));
                Q.push(triangle(r4, c4, r3, c3, r2, c2, d - 1));
                Q.push(triangle(r3, c3, r4, c4, r5, c5, d - 1));
            }
        }
};

uni::geocon::geocon(GLsizei d) : n((1 << d) + 1)
{
    GLsizei  m = n - 1;
    GLushort i = 0;

    // Allocate and initialize the mesh index buffer.

    index = new GLushort[n * n];

    memset(index, 0, n * n * sizeof (GLushort));

    index[0]         = i++;
    index[m]         = i++;
    index[m * m + m] = i++;

    // Perform a breadth-first subdivision of the triangular mesh.

    std::queue<triangle> Q;

    for (Q.push(triangle(0, 0, 0, m, m, 0, d)); !Q.empty(); Q.pop())
         Q.front().proc(index, n, i, Q);
}

uni::geocon::~geocon()
{
    delete [] index;
}

GLushort uni::geocon::get(GLsizei r, GLsizei c)
{
    return (r < n - c) ? index[r * n + c] : 0xFFFF;
}

//-----------------------------------------------------------------------------

void uni::geolut::subdiv(GLushort *p, GLsizei d,
                         GLsizei r0, GLsizei c0,
                         GLsizei r1, GLsizei c1,
                         GLsizei r2, GLsizei c2, geocon& con)
{
    if (d > 0)
    {
        // Compute the midpoint vertices of each edge of this triangle.

        GLsizei r3 = (r1 + r2) / 2, c3 = (c1 + c2) / 2;
        GLsizei r4 = (r2 + r0) / 2, c4 = (c2 + c0) / 2;
        GLsizei r5 = (r0 + r1) / 2, c5 = (c0 + c1) / 2;

        // Set the source operands for each edge bisection.

        p[con.get(r3, c3) * 2 + 0] = con.get(r1, c1);
        p[con.get(r3, c3) * 2 + 1] = con.get(r2, c2);

        p[con.get(r4, c4) * 2 + 0] = con.get(r2, c2);
        p[con.get(r4, c4) * 2 + 1] = con.get(r0, c0);

        p[con.get(r5, c5) * 2 + 0] = con.get(r0, c0);
        p[con.get(r5, c5) * 2 + 1] = con.get(r1, c1);

        // Recursively process all sub-triangles.

        subdiv(p, d - 1, r0, c0, r5, c5, r4, c4, con);
        subdiv(p, d - 1, r5, c5, r1, c1, r3, c3, con);
        subdiv(p, d - 1, r4, c4, r3, c3, r2, c2, con);
        subdiv(p, d - 1, r3, c3, r4, c4, r5, c5, con);
    }
}

uni::geolut::geolut(GLsizei d, geocon& con) :
    ogl::image(vtx_count(d), 1, GL_TEXTURE_RECTANGLE_ARB,
                                GL_LUMINANCE16_ALPHA16)
{
    GLsizei m = 1 << d;
    GLsizei c = vtx_count(d);

    // Initialize the geometry cache bisection lookup table.

    GLushort *p = new GLushort[c * 2];

    memset(p, 0, c * 2 * sizeof (GLushort));
    subdiv(p, d, 0, 0, 0, m, m, 0, con);

    // Copy the lookup table to the texture.

    bind(GL_TEXTURE0);
    {
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, c, 1,
                        GL_LUMINANCE_ALPHA, GL_UNSIGNED_SHORT, p);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }
    free(GL_TEXTURE0);

    OGLCK();

    delete [] p;
}

//-----------------------------------------------------------------------------

GLushort *uni::geoidx::append(GLushort *p, 
                              GLsizei r0, GLsizei c0,
                              GLsizei r1, GLsizei c1,
                              GLsizei r2, GLsizei c2, geocon& con)
{
    // Append a triangle to the index list.

    *(p++) = con.get(r0, c0);
    *(p++) = con.get(r1, c1);
    *(p++) = con.get(r2, c2);

    return p;
}

GLushort *uni::geoidx::subdiv(GLushort *p, GLsizei d,
                              GLsizei r0, GLsizei c0, int i,
                              GLsizei r1, GLsizei c1, int j,
                              GLsizei r2, GLsizei c2, int k, geocon& con)
{
    if (d > 0)
    {
        // Compute the midpoint vertices of each edge of this triangle.

        GLsizei r3 = (r1 + r2) / 2, c3 = (c1 + c2) / 2;
        GLsizei r4 = (r2 + r0) / 2, c4 = (c2 + c0) / 2;
        GLsizei r5 = (r0 + r1) / 2, c5 = (c0 + c1) / 2;

        if (d > 1)
        {
            // Recursively process all sub-triangles.

            p = subdiv(p, d - 1, r0, c0, i, r5, c5, 0, r4, c4, k, con);
            p = subdiv(p, d - 1, r5, c5, i, r1, c1, j, r3, c3, 0, con);
            p = subdiv(p, d - 1, r4, c4, 0, r3, c3, j, r2, c2, k, con);
            p = subdiv(p, d - 1, r3, c3, 0, r4, c4, 0, r5, c5, 0, con);
        }
        else
        {
            // The trivial case.

            if (i && j && k)
            {
                p = append(p, r0, c0, r1, c1, r2, c2, con);
            }

            // The corner cases.
            
            else if (k && i)
            {
                p = append(p, r0, c0, r1, c1, r3, c3, con);
                p = append(p, r0, c0, r3, c3, r2, c2, con);
            }
            else if (i && j)
            {
                p = append(p, r1, c1, r2, c2, r4, c4, con);
                p = append(p, r1, c1, r4, c4, r0, c0, con);
            }
            else if (j && k)
            {
                p = append(p, r2, c2, r0, c0, r5, c5, con);
                p = append(p, r2, c2, r5, c5, r1, c1, con);
            }

            // The edge cases.

            else if (i)
            {
                p = append(p, r0, c0, r1, c1, r4, c4, con);
                p = append(p, r4, c4, r1, c1, r3, c3, con);
                p = append(p, r4, c4, r3, c3, r2, c2, con);
            }
            else if (j)
            {
                p = append(p, r0, c0, r5, c5, r4, c4, con);
                p = append(p, r5, c5, r1, c1, r2, c2, con);
                p = append(p, r4, c4, r5, c5, r2, c2, con);
            }
            else if (k)
            {
                p = append(p, r0, c0, r5, c5, r3, c3, con);
                p = append(p, r5, c5, r1, c1, r3, c3, con);
                p = append(p, r0, c0, r3, c3, r2, c2, con);
            }

            // The default case.

            else
            {
                p = append(p, r0, c0, r5, c5, r4, c4, con);
                p = append(p, r5, c5, r1, c1, r3, c3, con);
                p = append(p, r4, c4, r3, c3, r2, c2, con);
                p = append(p, r3, c3, r4, c4, r5, c5, con);
            }
        }
    }
    else p = append(p, r0, c0, r1, c1, r2, c2, con);

    return p;
}

uni::geoidx::geoidx(GLsizei d, geocon& con) :
    ogl::buffer(GL_ELEMENT_ARRAY_BUFFER_ARB,
                tri_count(d) * 3 * 8 * sizeof (GLushort), GL_STATIC_DRAW_ARB)
{
    GLsizei s = 3 * tri_count(d);
    GLsizei m = 1 << d;

    GLushort *p;

    bind();
    {
        if ((p = (GLushort *) wmap()))
        {
            memset(p, 0, 8 * s * sizeof (GLushort));

            for (int i = 0; i < 8; i += 4)
                for (int j = 0; j < 4; j += 2)
                    for (int k = 0; k < 2; k += 1)
                    {
                        GLushort *q = p + s * (i + j + k);

                        subdiv(q, d, 0, 0, i, 0, m, j, m, 0, k, con);
                    }
            umap();
        }
    }
    free();
}

//-----------------------------------------------------------------------------

double uni::georad::func(double x)
{
    return tan(x * PI / 4);
}

// TODO: understand the edge behaviour of this mapping.
// GPU Gems 2 chapter 24

uni::georad::georad(GLsizei n) :
    lut(n, GL_TEXTURE_1D, GL_LUMINANCE16, GL_LUMINANCE)
{
    GLushort *p = new GLushort[n];
    GLsizei   i;

    // Compute the radius correction function lookup table.

    for (i = 0; i < n; ++i)
        p[i] = GLushort(65535 * func(double(i) / double(n - 1)));

    // Copy the table to the texture.

    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, n,
                    GL_LUMINANCE, GL_UNSIGNED_SHORT, p);
    OGLCK();

    delete [] p;
}

//-----------------------------------------------------------------------------

uni::geodat::geodat(GLsizei d) :
    d(d),
    _rad( ),
    _con(d),
    _lut(d, _con),
    _idx(d, _con)
{
}

GLsizei uni::geodat::vtx_len() const
{
    return vtx_count(d);
}

GLsizei uni::geodat::idx_len() const
{
    return tri_count(d);
}

//=============================================================================

uni::geobuf::geobuf(GLsizei w, GLsizei h, std::string rect_vert,
                                          std::string copy_frag,
                                          std::string calc_frag,
                                          std::string show_frag) :

    ping(glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                               GL_FLOAT_RGBA32_NV, false)),
    pong(glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                               GL_FLOAT_RGBA32_NV, false)),
    src(ping),
    dst(pong),

    w(w), h(h),

    copy(glob->load_program(rect_vert, copy_frag)),
    calc(glob->load_program(rect_vert, calc_frag)),
    show(glob->load_program(rect_vert, show_frag))
{
    // Initialize shader uniforms.

    copy->bind();
    {
        copy->uniform("src", 0);
    }
    copy->free();

    calc->bind();
    {
        calc->uniform("src", 0);
    }
    calc->free();
    
    show->bind();
    {
        show->uniform("src", 0);
    }
    show->free();
}

uni::geobuf::~geobuf()
{
    glob->free_program(show);
    glob->free_program(calc);
    glob->free_program(copy);

    glob->free_frame(pong);
    glob->free_frame(ping);
}

void uni::geobuf::draw() const
{
    // Draw the output buffer.
/*
    show->bind();
    src->draw();
    show->free();
*/
    std::cerr << "geobuff::draw removed" << std::endl;
}

void uni::geobuf::null() const
{
/*
    ping->null();
    pong->null();
*/
    std::cerr << "geobuff::null removed" << std::endl;
}

void uni::geobuf::swap()
{
    // Ping-pong the render targets.

    ogl::frame *tmp = src;
    src             = dst;
    dst             = tmp;
}

void uni::geobuf::bind_proc() const
{
    src->bind_color();
    dst->bind(true);

    calc->bind();
}

void uni::geobuf::free_proc() const
{
    calc->free();

    dst->free(true);
    src->free_color();
}

//-----------------------------------------------------------------------------

uni::geogen::geogen(GLsizei d, GLsizei h, std::string rect_vert,
                                          std::string copy_frag,
                                          std::string calc_frag,
                                          std::string show_frag) :

    geobuf(vtx_count(d), h, rect_vert,
                            copy_frag,
                            calc_frag,
                            show_frag),

    d(d), p(0), buff(GL_PIXEL_UNPACK_BUFFER_ARB, 3 * h * 4 * sizeof (GLfloat))
{
    // geobuf generators use the geolut texture coordinate lookup table.

    calc->bind();
    {
        calc->uniform("lut", 1);
        calc->uniform("siz", 1024.0f);
    }
    calc->free();
}

void uni::geogen::init()
{
    // Map the seed vector buffer.

    buff.bind();
    buff.zero();

    p = (GLfloat *) buff.wmap();

    buff.free();
}

void uni::geogen::seed(GLsizei i, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    // Copy the given vector to the seed buffer.

    if (p && i < h * 3)
    {
        p[4 * i + 0] = x;
        p[4 * i + 1] = y;
        p[4 * i + 2] = z;
        p[4 * i + 3] = w;
    }
}

void uni::geogen::fini(GLsizei c)
{
    // Copy the seed vector buffer to the input buffer.

    buff.bind();
    buff.umap();

    bind(GL_TEXTURE0);
    {
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB,
                        0, 0, 0, 3, c, GL_RGBA, GL_FLOAT, 0);
    }
    free(GL_TEXTURE0);

    buff.free();
}

void uni::geogen::proc(GLsizei c)
{
    // Perform a breadth-first traversal of the triangle subdivision.

    for (GLsizei x0 = 0, x1 = 3, i = 0; i < d; ++i)
    {
        GLsizei x2 = vtx_count(i + 1);

        bind_proc();
        {
            copy->bind();
            glRecti(GLint(x0), 0, GLint(x1), GLint(c));
            copy->free();

            calc->bind();
            glRecti(GLint(x1), 0, GLint(x2), GLint(c));
            calc->free();
        }
        free_proc();

        // Step down the buffer.

        x0 = x1;
        x1 = x2;

        swap();
    }
}

//-----------------------------------------------------------------------------

uni::geonrm::geonrm(GLsizei d, GLsizei h) :

    uni::geogen(d, h, "glsl/trivial.vert",
                      "glsl/copybuf.frag",
                      "glsl/calcnrm.frag",
                      "glsl/shownrm.frag")
{
}

//-----------------------------------------------------------------------------

uni::geopos::geopos(GLsizei d, GLsizei h) :

    uni::geogen(d, h, "glsl/trivial.vert",
                      "glsl/copybuf.frag",
                      "glsl/calcpos.frag",
                      "glsl/showpos.frag")
{
    calc->bind();
    {
        calc->uniform("nrm", 2);
        calc->uniform("rad", 3);
    }
    calc->free();
}

void uni::geopos::init()
{
    geogen::init();

    // Initialize the bounding volume accumulator.

    b[0] =  std::numeric_limits<GLfloat>::max();
    b[1] = -std::numeric_limits<GLfloat>::max();
    b[2] =  std::numeric_limits<GLfloat>::max();
    b[3] = -std::numeric_limits<GLfloat>::max();
    b[4] =  std::numeric_limits<GLfloat>::max();
    b[5] = -std::numeric_limits<GLfloat>::max();

    d0 =  std::numeric_limits<GLfloat>::max();
    d1 = -std::numeric_limits<GLfloat>::max();
}

void uni::geopos::seed(GLsizei i, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    geogen::seed(i, x, y, z, w);

    // Accumulate the bounding volume.

    b[0] = std::min(x, b[0]);
    b[1] = std::max(x, b[1]);
    b[2] = std::min(y, b[2]);
    b[3] = std::max(y, b[3]);

    if (z < 0)
    {
        b[4] = std::min(b[4], -z);
        b[5] = std::max(b[5], -z);
    }

    GLfloat d = GLfloat(sqrt(x * x + y * y + z * z));

    d0 = std::min(d0, d);
    d1 = std::max(d1, d);
}

void uni::geopos::fini(GLsizei c)
{
    geogen::fini(c);

    // Apply the bounding volume.

    show->bind();
    show->uniform("range", GLfloat(std::max(std::max(fabs(b[0]),
                                                     fabs(b[1])),
                                            std::max(fabs(b[3]),
                                                     fabs(b[4])))));
    show->free();
}

//-----------------------------------------------------------------------------

uni::geotex::geotex(GLsizei d, GLsizei h) :

    uni::geogen(d, h, "glsl/trivial.vert",
                      "glsl/copybuf.frag",
                      "glsl/calctex.frag",
                      "glsl/showtex.frag")
{
}

//-----------------------------------------------------------------------------

uni::geoacc::geoacc(GLsizei d, GLsizei h) :

    uni::geobuf(vtx_count(d), h, "glsl/trivial.vert",
                                 "glsl/copyacc.frag",
                                 "glsl/calcacc.frag",
                                 "glsl/showpos.frag")
{
    copy->bind();
    {
        copy->uniform("pos", 2);
    }
    copy->free();

    calc->bind();
    {
        calc->uniform("src", 0);
        calc->uniform("map", 1);
        calc->uniform("pos", 2);
        calc->uniform("nrm", 3);
        calc->uniform("tex", 4);
    }
    calc->free();
}

void uni::geoacc::init(GLsizei c)
{
    // Initialize the ZR min/max using the accumulation buffer.

    dst->bind(true);
    copy->bind();
    {
         glRecti(0, 0, GLint(w), GLint(c));
    }
    copy->free();
    dst->free(true);
}

//-----------------------------------------------------------------------------

uni::geoext::geoext(GLsizei d, GLsizei h) :

    uni::geobuf(vtx_count(d), h, "glsl/trivial.vert",
                                 "glsl/copyext.frag",
                                 "glsl/calcext.frag",
                                 "glsl/showpos.frag"),

    buff(GL_PIXEL_PACK_BUFFER_ARB, 4 * h * sizeof (GLfloat))
{
    copy->bind();
    {
        copy->uniform("pos", 1);
    }
    copy->free();
}

void uni::geoext::proc(GLsizei c)
{
    // Initialize the ZR min/max using the accumulation buffer.

    bind_proc();
    {
        copy->bind();
        glRecti(0, 0, GLint(w), GLint(c));
        copy->free();
    }
    free_proc();

    swap();

    // Performa a parallel reduction of the ZR min/max buffer.

    for (GLsizei W = w; W > 1; W /= 2)
    {
        bind_proc();
        {
            GLint W2 = GLint((W % 2 == 0) ? W / 2 : W / 2 + 1);

            calc->uniform("siz", GLfloat(W2), 0.0f);

            glRecti(0, 0, W2, GLint(c));
        }
        free_proc();

        swap();
    }

    // Copy the results of the reduction to the output buffer.

    bind_proc();
    buff.bind();
    {
        glReadPixels(0, 0, 1, c, GL_RGBA, GL_FLOAT, 0);
    }
    buff.free();
    free_proc();
}

const GLfloat *uni::geoext::rmap() const
{
    const GLfloat *p;

    buff.bind();
    p = (GLfloat *) buff.rmap();
    buff.free();

    return p;
}

void uni::geoext::umap() const
{
    buff.bind();
    buff.umap();
    buff.free();
}

//-----------------------------------------------------------------------------

uni::geovtx::geovtx(GLsizei d, GLsizei h) :
    ogl::buffer(GL_ARRAY_BUFFER_ARB, vtx_count(d) * h * 12 * sizeof (GLfloat)),
    w(vtx_count(d)),
    h(h)
{
}

void uni::geovtx::read_v(GLsizei c)
{
    bind(GL_PIXEL_PACK_BUFFER_ARB);
    {
        glReadPixels(0, 0, w, c, GL_RGBA, GL_FLOAT, 0);
    }
    free(GL_PIXEL_PACK_BUFFER_ARB);
}

void uni::geovtx::read_n(GLsizei c)
{
    GLvoid *O = (GLvoid *) (w * h * 4 * sizeof (GLfloat));

    bind(GL_PIXEL_PACK_BUFFER_ARB);
    {
        glReadPixels(0, 0, w, c, GL_RGBA, GL_FLOAT, O);
    }
    free(GL_PIXEL_PACK_BUFFER_ARB);
}

void uni::geovtx::read_c(GLsizei c)
{
    GLvoid *O = (GLvoid *) (w * h * 8 * sizeof (GLfloat));

    bind(GL_PIXEL_PACK_BUFFER_ARB);
    {
        glReadPixels(0, 0, w, c, GL_RGBA, GL_FLOAT, O);
    }
    free(GL_PIXEL_PACK_BUFFER_ARB);
}

//-----------------------------------------------------------------------------
