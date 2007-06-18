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

#include <cmath>
#include <memory>

#include "matrix.hpp"
#include "opengl.hpp"
#include "mesh.hpp"
#include "glob.hpp"

//-----------------------------------------------------------------------------

ogl::aabb::aabb()
{
    a[0] = +std::numeric_limits<GLfloat>::max();
    a[1] = +std::numeric_limits<GLfloat>::max();
    a[2] = +std::numeric_limits<GLfloat>::max();

    z[0] = -std::numeric_limits<GLfloat>::max();
    z[1] = -std::numeric_limits<GLfloat>::max();
    z[2] = -std::numeric_limits<GLfloat>::max();
}

ogl::aabb::aabb(const aabb& that, const GLfloat *M)
{
}

void ogl::aabb::merge(const GLfloat *p)
{
    a[0] = std::min(a[0], p[0]);
    a[1] = std::min(a[1], p[1]);
    a[2] = std::min(a[2], p[2]);

    z[0] = std::max(z[0], p[0]);
    z[1] = std::max(z[1], p[1]);
    z[2] = std::max(z[2], p[2]);
}

void ogl::aabb::merge(const aabb& that)
{
    a[0] = std::min(a[0], that.a[0]);
    a[1] = std::min(a[1], that.a[1]);
    a[2] = std::min(a[2], that.a[2]);

    z[0] = std::max(z[0], that.z[0]);
    z[1] = std::max(z[1], that.z[1]);
    z[2] = std::max(z[2], that.z[2]);
}

bool ogl::aabb::test_plane(const GLfloat *M, const GLfloat *V)
{
    GLfloat P[4];

    mult_mat_vec(P, M, V);

    if (P[0] > 0)
        if (P[1] > 0)
            if (P[2] > 0)
                return (z[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3] > 0);
            else
                return (z[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3] > 0);
        else
            if (P[2] > 0)
                return (z[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3] > 0);
            else
                return (z[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3] > 0);
    else
        if (P[1] > 0)
            if (P[2] > 0)
                return (a[0] * P[0] + z[1] * P[1] + z[2] * P[2] + P[3] > 0);
            else
                return (a[0] * P[0] + z[1] * P[1] + a[2] * P[2] + P[3] > 0);
        else
            if (P[2] > 0)
                return (a[0] * P[0] + a[1] * P[1] + z[2] * P[2] + P[3] > 0);
            else
                return (a[0] * P[0] + a[1] * P[1] + a[2] * P[2] + P[3] > 0);
}

bool ogl::aabb::test(const GLfloat *M, const GLfloat *V)
{
    if (!test_plane(M, V +  0)) return false;
    if (!test_plane(M, V +  4)) return false;
    if (!test_plane(M, V +  8)) return false;
    if (!test_plane(M, V + 12)) return false;
    if (!test_plane(M, V + 16)) return false;

    return true;
}

//-----------------------------------------------------------------------------

ogl::mesh::mesh(std::string& name) :
    material(glob->load_binding(name)),
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

        normalize(t);

        // Accumulate the vertex tangent vectors.

        GLfloat *ti = tv[fi->i].v;
        GLfloat *tj = tv[fi->j].v;
        GLfloat *tk = tv[fi->k].v;

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

    // Normalize all tangent vectors.

    for (vec3_v::iterator ti = tv.begin(); ti != tv.end(); ++ti)
        normalize(ti->v);
}

//-----------------------------------------------------------------------------

void ogl::mesh::add_vert(vec3& v, vec3& n, vec2& u)
{
    vec3 t;

    // Add a new vertex.  Note position bounds.

    vv.push_back(v);
    nv.push_back(n);
    tv.push_back(t);
    uv.push_back(u);

    bound.merge(v.v);

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

void ogl::mesh::cache_verts(const ogl::mesh *that, const float *M,
                                                   const float *I)
{
    const GLsizei n = that->vv.size();

    // Cache that mesh's transformed vertices here.  Update bounding volume.

    vv.resize(n);
    nv.resize(n);
    tv.resize(n);
    uv.resize(n);

    bound = aabb();

    for (GLsizei i = 0; i < n; ++i)
    {
        mult_mat_pos(vv[i].v, M, that->vv[i].v);
        mult_xps_pos(nv[i].v, I, that->nv[i].v);
        mult_xps_pos(tv[i].v, I, that->tv[i].v);

        uv[i] = that->uv[i];

        bound.merge(vv[i].v);
    }

    dirty_verts = true;
}

void ogl::mesh::cache_faces(const ogl::mesh *that, GLuint d)
{
    const GLsizei n = that->faces.size();

    // Cache that mesh's offset face elements here.

    faces.resize(n);

    for (GLsizei i = 0; i < n; ++i)
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
    const GLsizei n = that->lines.size();

    // Cache that mesh's offset line elements here.

    lines.resize(n);

    for (GLsizei i = 0; i < n; ++i)
        lines[i] = line(that->lines[i].i + d,
                        that->lines[i].j + d);

    // Cache the offset element range.

    min = that->min + d;
    max = that->max + d;

    dirty_lines = true;
}

//-----------------------------------------------------------------------------

void ogl::mesh::buffv(GLfloat *v, GLfloat *n, GLfloat *t, GLfloat *u)
{
    // Copy all cached vertex data to the bound array buffer object.

    if (dirty_verts)
    {
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, GLintptrARB(v),
                           vv.size() * sizeof (vec3), &vv.front());
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, GLintptrARB(n),
                           nv.size() * sizeof (vec3), &nv.front());
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, GLintptrARB(t),
                           tv.size() * sizeof (vec3), &tv.front());
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, GLintptrARB(u),
                           uv.size() * sizeof (vec2), &uv.front());

        OGLCK();
    }
    dirty_verts = false;
}

void ogl::mesh::buffe(GLuint *e)
{
    // Copy all cached index data to the bound element array buffer object.

    if (dirty_faces && faces.size())
        glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GLintptrARB(e),
                           faces.size() * sizeof (face), &faces.front());

    e += faces.size() * 3;

    if (dirty_lines && lines.size())
        glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GLintptrARB(e),
                           lines.size() * sizeof (line), &lines.front());

    OGLCK();

    dirty_faces = false;
    dirty_lines = false;
}

//-----------------------------------------------------------------------------
