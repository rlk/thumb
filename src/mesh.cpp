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
#include <memory>

#include "matrix.hpp"
#include "opengl.hpp"
#include "mesh.hpp"
#include "glob.hpp"

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

static void mult_mat_GLvec3(GLfloat *v, const double *M, const GLfloat *u)
{
    double vv[3];
    double uu[3];

    // Transform a GL vector using a double matrix.

    uu[0] = double(u[0]);
    uu[1] = double(u[1]);
    uu[2] = double(u[2]);

    mult_mat_vec3(vv, M, uu);

    v[0] = GLfloat(vv[0]);
    v[1] = GLfloat(vv[1]);
    v[2] = GLfloat(vv[2]);
}

static void mult_xps_GLvec3(GLfloat *v, const double *I, const GLfloat *u)
{
    double vv[3];
    double uu[3];

    // Transform a GL vector using a double transpose.

    uu[0] = double(u[0]);
    uu[1] = double(u[1]);
    uu[2] = double(u[2]);

    mult_xps_vec3(vv, I, uu);

    v[0] = GLfloat(vv[0]);
    v[1] = GLfloat(vv[1]);
    v[2] = GLfloat(vv[2]);
}

//-----------------------------------------------------------------------------

void ogl::mesh::cache_verts(const ogl::mesh *that, const double *M,
                                                   const double *I)
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
        mult_mat_GLvec3(vv[i].v, M, that->vv[i].v);
        mult_xps_GLvec3(nv[i].v, I, that->nv[i].v);
        mult_xps_GLvec3(tv[i].v, I, that->tv[i].v);

        uv[i] = that->uv[i];

        bound.merge(double(vv[i].v[0]),
                    double(vv[i].v[1]),
                    double(vv[i].v[2]));
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
