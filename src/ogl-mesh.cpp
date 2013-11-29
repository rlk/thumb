//  Copyright (C) 2007-2011 Robert Kooima
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

#include <cmath>
#include <cassert>

#include <etc-math.hpp>
#include <ogl-opengl.hpp>
#include <ogl-mesh.hpp>
#include <app-glob.hpp>

//-----------------------------------------------------------------------------

ogl::mesh::mesh(std::string& name) :
    material(glob->load_binding(name, "default")),
    min(std::numeric_limits<GLuint>::max()),
    max(std::numeric_limits<GLuint>::min()),
    dirty_verts(false),
    dirty_faces(false),
    dirty_lines(false)
{
}

ogl::mesh::mesh() :
    material(0),
    min(std::numeric_limits<GLuint>::max()),
    max(std::numeric_limits<GLuint>::min()),
    dirty_verts(false),
    dirty_faces(false),
    dirty_lines(false)
{
}

ogl::mesh::~mesh()
{
    if (material) glob->free_binding(material);
}

//-----------------------------------------------------------------------------

void ogl::mesh::apply_offset(const double *d)
{
    for (GLvec3_v::iterator i = vv.begin(); i != vv.end(); ++i)
    {
        i->v[0] += GLfloat(d[0]);
        i->v[1] += GLfloat(d[1]);
        i->v[2] += GLfloat(d[2]);
    }
}

void ogl::mesh::calc_tangent()
{
    // Assume all tangent vectors were zeroed during loading.  Enumerate faces.

    for (ogl::face_i fi = faces.begin(); fi != faces.end(); ++fi)
    {
        // Compute the vertex position differences.

        const GLfloat *vi = vv[fi->i].v;
        const GLfloat *vj = vv[fi->j].v;
        const GLfloat *vk = vv[fi->k].v;

        GLfloat dv0[3], dv1[3];

        dv0[0] = vj[0] - vi[0];
        dv0[1] = vj[1] - vi[1];
        dv0[2] = vj[2] - vi[2];

        dv1[0] = vk[0] - vi[0];
        dv1[1] = vk[1] - vi[1];
        dv1[2] = vk[2] - vi[2];

        // Compute the vertex texture coordinate differences.

        const GLfloat *si = uv[fi->i].v;
        const GLfloat *sj = uv[fi->j].v;
        const GLfloat *sk = uv[fi->k].v;

        GLfloat ds0[2], ds1[2];

        ds0[0] = sj[0] - si[0];
        ds0[1] = sj[1] - si[1];

        ds1[0] = sk[0] - si[0];
        ds1[1] = sk[1] - si[1];

        // Compute the tangent vector.

        GLfloat t[3];

        t[0] = ds1[1] * dv0[0] - ds0[1] * dv1[0];
        t[1] = ds1[1] * dv0[1] - ds0[1] * dv1[1];
        t[2] = ds1[1] * dv0[2] - ds0[1] * dv1[2];

        // Accumulate the vertex tangent vectors.

        if (DOT3(t, t) > 0.0f)
        {
            GLfloat *ti = tv[fi->i].v;
            GLfloat *tj = tv[fi->j].v;
            GLfloat *tk = tv[fi->k].v;

            normalize(t);

            ti[0] += t[0];
            ti[1] += t[1];
            ti[2] += t[2];

            tj[0] += t[0];
            tj[1] += t[1];
            tj[2] += t[2];

            tk[0] += t[0];
            tk[1] += t[1];
            tk[2] += t[2];
        }
    }

    // Orthonormalize all tangent vectors.

    assert(tv.size() == nv.size());

    for (unsigned int i = 0; i < tv.size(); ++i)
    {
        GLfloat b[3];

        crossprod(b, nv[i].v, tv[i].v);
        normalize(b);

        crossprod(tv[i].v, b, nv[i].v);
        normalize(tv[i].v);
    }
}

//-----------------------------------------------------------------------------

void ogl::mesh::add_vert(GLvec3& v, GLvec3& n, GLvec2& u)
{
    GLvec3 t;

    // Add a new vertex.  Note position bounds.

    vv.push_back(v);
    nv.push_back(n);
    tv.push_back(t);
    uv.push_back(u);

    bound.merge(double(v.v[0]),
                double(v.v[1]),
                double(v.v[2]));

    dirty_verts = true;
}

void ogl::mesh::add_face(GLuint i, GLuint j, GLuint k)
{
    // Add a new face.  Note index range.

    faces.push_back(face(i, j, k));

    min = std::min(std::min(min, i), std::min(j, k));
    max = std::max(std::max(max, i), std::max(j, k));

    dirty_faces = true;
}

void ogl::mesh::add_line(GLuint i, GLuint j)
{
    // Add a new line.  Note index range.

    lines.push_back(line(i, j));

    min = std::min(min, std::min(i, j));
    max = std::max(max, std::max(i, j));

    dirty_lines = true;
}

//-----------------------------------------------------------------------------

static void transform_GLvec3(GLfloat *v, const mat4& M, const GLfloat *u)
{
    // Transform a GL vector using a double matrix.

    vec3 t = M * vec3(double(u[0]),
                      double(u[1]),
                      double(u[2]));

    v[0] = GLfloat(t[0]);
    v[1] = GLfloat(t[1]);
    v[2] = GLfloat(t[2]);
}

//-----------------------------------------------------------------------------

void ogl::mesh::cache_verts(const ogl::mesh *that, const mat4& M,
                                                   const mat4& I)
{
    const size_t n = that->vv.size();

    // Cache that mesh's transformed vertices here.  Update bounding volume.

    vv.resize(n);
    nv.resize(n);
    tv.resize(n);
    uv.resize(n);

    bound = aabb();

    for (size_t i = 0; i < n; ++i)
    {
        transform_GLvec3(vv[i].v,           M,  that->vv[i].v);
        transform_GLvec3(nv[i].v, transpose(I), that->nv[i].v);
        transform_GLvec3(tv[i].v, transpose(I), that->tv[i].v);

        uv[i] = that->uv[i];

        bound.merge(double(vv[i].v[0]),
                    double(vv[i].v[1]),
                    double(vv[i].v[2]));
    }

    dirty_verts = true;
}

void ogl::mesh::cache_faces(const ogl::mesh *that, GLuint d)
{
    const size_t n = that->faces.size();

    // Cache that mesh's offset face elements here.

    faces.resize(n);

    for (size_t i = 0; i < n; ++i)
        faces[i] = face(that->faces[i].i + d,
                        that->faces[i].j + d,
                        that->faces[i].k + d);

    // Cache the offset element range.

    min = that->min + d;
    max = that->max + d;

    dirty_faces = true;
}

void ogl::mesh::cache_lines(const ogl::mesh *that, GLuint d)
{
    const size_t n = that->lines.size();

    // Cache that mesh's offset line elements here.

    lines.resize(n);

    for (size_t i = 0; i < n; ++i)
        lines[i] = line(that->lines[i].i + d,
                        that->lines[i].j + d);

    // Cache the offset element range.

    min = that->min + d;
    max = that->max + d;

    dirty_lines = true;
}

//-----------------------------------------------------------------------------

static void buffer(GLintptr off, GLsizeiptr siz, const GLvoid *dat)
{
    const GLubyte   *buf = (const GLubyte *) dat;
    const GLsizeiptr max = 1024 * 256;

    // This works around an apparent bug under OSX 10.5.6 (using an NVIDIA
    // GeForce 8800GT) that corrupts uploads larger than about a megabyte.

    while (siz > 0)
    {
        GLsizeiptr len = std::min(siz, max);

        glBufferSubData(GL_ARRAY_BUFFER, off, len, buf);

        off += len;
        buf += len;
        siz -= len;
    }
}

void ogl::mesh::buffv(const GLfloat *v,
                      const GLfloat *n,
                      const GLfloat *t,
                      const GLfloat *u)
{
    // Copy all cached vertex data to the bound array buffer object.

    if (dirty_verts)
    {
        buffer(GLintptr(v), vv.size() * sizeof (GLvec3), &vv.front());
        buffer(GLintptr(n), nv.size() * sizeof (GLvec3), &nv.front());
        buffer(GLintptr(t), tv.size() * sizeof (GLvec3), &tv.front());
        buffer(GLintptr(u), uv.size() * sizeof (GLvec2), &uv.front());
    }
    dirty_verts = false;
}

void ogl::mesh::buffe(const GLuint *e)
{
    // Copy all cached index data to the bound element array buffer object.

    if (dirty_faces && faces.size())
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(e),
                           faces.size() * sizeof (face), &faces.front());

    e += faces.size() * 3;

    if (dirty_lines && lines.size())
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(e),
                           lines.size() * sizeof (line), &lines.front());

    dirty_faces = false;
    dirty_lines = false;
}

//-----------------------------------------------------------------------------
