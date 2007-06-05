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
    min[0] = std::numeric_limits<GLfloat>::max();
    min[1] = std::numeric_limits<GLfloat>::max();
    min[2] = std::numeric_limits<GLfloat>::max();

    max[0] = std::numeric_limits<GLfloat>::min();
    max[1] = std::numeric_limits<GLfloat>::min();
    max[2] = std::numeric_limits<GLfloat>::min();

    rad[0] = std::numeric_limits<GLfloat>::min();
}

void ogl::aabb::merge(const GLfloat *p)
{
    min[0] = std::min(min[0], p[0]);
    min[1] = std::min(min[1], p[1]);
    min[2] = std::min(min[2], p[2]);

    max[0] = std::max(max[0], p[0]);
    max[1] = std::max(max[1], p[1]);
    max[2] = std::max(max[2], p[2]);

    float r = float(sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]));

    rad[0] = std::max(rad[0], r);
}

void ogl::aabb::merge(const aabb& that)
{
    min[0] = std::min(min[0], that.min[0]);
    min[1] = std::min(min[1], that.min[1]);
    min[2] = std::min(min[2], that.min[2]);

    max[0] = std::max(max[0], that.max[0]);
    max[1] = std::max(max[1], that.max[1]);
    max[2] = std::max(max[2], that.max[2]);

    rad[0] = std::max(rad[0], that.rad[0]);
}

//-----------------------------------------------------------------------------

ogl::mesh::mesh(std::string& name) : material(glob->load_binding(name))
{
}

ogl::mesh::mesh() : material(0)
{
}

ogl::mesh::~mesh()
{
    glob->free_binding(material);
}

//-----------------------------------------------------------------------------

void ogl::mesh::calc_tangent()
{
    // Assume all tangent vectors were zeroed during loading.  Enumerate faces.

    for (ogl::face_i fi = faces.begin(); fi != faces.end(); ++fi)
    {
        // Compute the vertex position differences.

        const float *vi = verts[fi->i].v.v;
        const float *vj = verts[fi->j].v.v;
        const float *vk = verts[fi->k].v.v;

        float dv0[3], dv1[3];

        dv0[0] = vj[0] - vi[0];
        dv0[1] = vj[1] - vi[1];
        dv0[2] = vj[2] - vi[2];

        dv1[0] = vk[0] - vi[0];
        dv1[1] = vk[1] - vi[1];
        dv1[2] = vk[2] - vi[2];

        // Compute the vertex texture coordinate differences.

        const float *si = verts[fi->i].s.v;
        const float *sj = verts[fi->j].s.v;
        const float *sk = verts[fi->k].s.v;

        float ds0[2], ds1[2];

        ds0[0] = sj[0] - si[0];
        ds0[1] = sj[1] - si[1];

        ds1[0] = sk[0] - si[0];
        ds1[1] = sk[1] - si[1];

        // Compute the tangent vector.

        float t[3];

        t[0] = ds1[1] * dv0[0] - ds0[1] * dv1[0];
        t[1] = ds1[1] * dv0[1] - ds0[1] * dv1[1];
        t[2] = ds1[1] * dv0[2] - ds0[1] * dv1[2];

        normalize(t);

        // Accumulate the vertex tangent vectors.

        float *ti = verts[fi->i].t.v;
        float *tj = verts[fi->j].t.v;
        float *tk = verts[fi->k].t.v;

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

    for (ogl::vert_i vi = verts.begin(); vi != verts.end(); ++vi)
        normalize(vi->t.v);
}

//-----------------------------------------------------------------------------

void ogl::mesh::cache_verts(const ogl::mesh& that, const float *M,
                                                   const float *I)
{
    // Cache that mesh's transformed vertices here.

    // Update the bounding volume.
}

//-----------------------------------------------------------------------------

void ogl::mesh::merge_bound(GLfloat *bound_min,
                            GLfloat *bound_max,
                            GLfloat *bound_rad)
{
    // Merge this mesh's bounding volume with the given one.

    bound_min[0] = std::min(bound_min[0], this->bound_min[0]);
    bound_min[1] = std::min(bound_min[1], this->bound_min[1]);
    bound_min[2] = std::min(bound_min[2], this->bound_min[2]);

    bound_max[0] = std::max(bound_max[0], this->bound_max[0]);
    bound_max[1] = std::max(bound_max[1], this->bound_max[1]);
    bound_max[2] = std::max(bound_max[2], this->bound_max[2]);

    bound_rad[0] = std::max(bound_rad[0], this->bound_rad[0]);
}

//-----------------------------------------------------------------------------
/*
ogl::vert_p ogl::mesh::vert_cache(ogl::vert_p o, const GLfloat *M,
                                                 const GLfloat *I) const
{
    const size_t n = verts.size();

    // Acquire a temporary vertex buffer.

    if (vert_p temp = new vert[n])
    {
        // Pretransform all vertices to the temporary buffer.

        for (size_t i = 0; i < n; ++i)
        {
            mult_mat_pos(temp[i].v.v, M, verts[i].v.v);
            mult_xps_pos(temp[i].n.v, I, verts[i].n.v);
            mult_xps_pos(temp[i].t.v, I, verts[i].t.v);

            temp[i].s = verts[i].s;
        }

        // Upload the pretransformed vertices to the current vertex buffer.

        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, GLintptrARB(o),
                           n * sizeof (vert), temp);
        OGLCK();

        delete [] temp;
    }

    // Walk the offset down the vertex array.

    return o + n;
}

GLuint *ogl::mesh::face_cache(GLuint *o, GLuint d, GLuint& min,
                                                   GLuint& max) const
{
    const size_t n = faces.size();

    min = std::numeric_limits<GLuint>::max();
    max = std::numeric_limits<GLuint>::min();

    // Acquire a temporary element buffer.

    if (face_p temp = new face[n])
    {
        // Offset all elements to the temporary buffer.  Find extrema.

        for (size_t i = 0; i < n; ++i)
        {
            temp[i].i = faces[i].i + d;
            temp[i].j = faces[i].j + d;
            temp[i].k = faces[i].k + d;

            min = std::min(std::min(min,       temp[i].i),
                           std::min(temp[i].j, temp[i].k));
            max = std::max(std::max(max,       temp[i].i),
                           std::max(temp[i].j, temp[i].k));
        }

        // Upload the offset elements to the current element buffer.

        glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GLintptrARB(o),
                           n * sizeof (face), temp);
        OGLCK();

        delete [] temp;
    }

    // Walk the pointer offset down the element array.

    return o + n * 3;
}

GLuint *ogl::mesh::line_cache(GLuint *o, GLuint d, GLuint& min,
                                                   GLuint& max) const
{
    const size_t n = lines.size();

    min = std::numeric_limits<GLuint>::max();
    max = std::numeric_limits<GLuint>::min();

    // Acquire a temporary element buffer.

    if (line_p temp = new line[n])
    {
        // Offset all elements to the temporary buffer.  Find extrema

        for (size_t i = 0; i < n; ++i)
        {
            temp[i].i = lines[i].i + d;
            temp[i].j = lines[i].j + d;

            min = std::min(min, std::min(temp[i].i, temp[i].j));
            max = std::max(max, std::max(temp[i].i, temp[i].j));
        }

        // Upload the offset elements to the current element buffer.

        glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GLintptrARB(o),
                           n * sizeof (line), temp);
        OGLCK();

        delete [] temp;
    }

    // Walk the pointer offset down the element array.

    return o + n * 2;
}
*/
//-----------------------------------------------------------------------------
/*
void ogl::mesh::box_bound(GLfloat *b) const
{
    b[0] = std::numeric_limits<GLfloat>::max();
    b[1] = std::numeric_limits<GLfloat>::max();
    b[2] = std::numeric_limits<GLfloat>::max();
    b[3] = std::numeric_limits<GLfloat>::min();
    b[4] = std::numeric_limits<GLfloat>::min();
    b[5] = std::numeric_limits<GLfloat>::min();

    for (vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        b[0] = std::min(b[0], vi->v.v[0]);
        b[1] = std::min(b[1], vi->v.v[1]);
        b[2] = std::min(b[2], vi->v.v[2]);
        b[3] = std::max(b[3], vi->v.v[0]);
        b[4] = std::max(b[4], vi->v.v[1]);
        b[5] = std::max(b[5], vi->v.v[2]);
    }
}

void ogl::mesh::sph_bound(GLfloat *b) const
{
    b[0] = std::numeric_limits<GLfloat>::min();

    for (vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        GLfloat r = sqrt(vi->v.v[0] * vi->v.v[0] +
                         vi->v.v[1] * vi->v.v[1] +
                         vi->v.v[2] * vi->v.v[2]);

        b[0] = std::max(b[0], r);
    }
}
*/
//-----------------------------------------------------------------------------
