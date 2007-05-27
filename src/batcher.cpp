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

#include "mesh.hpp"
#include "glob.hpp"
#include "matrix.hpp"
#include "opengl.hpp"
#include "batcher.hpp"

//=============================================================================
// Batchable element

ogl::element::element(const element& that) :
    status(false), srf(glob->dupe_surface(that.srf))
{
    load_mat(M, that.M);

    // Build the vertex array offset map using the named surface.

    for (GLsizei i = 0; i < srf->max_mesh(); ++i)
        off[srf->get_mesh(i)] = 0;
}

ogl::element::element(std::string name) :
    status(false), srf(glob->load_surface(name))
{
    load_idt(M);

    // Build the vertex array offset map using the named surface.

    for (GLsizei i = 0; i < srf->max_mesh(); ++i)
        off[srf->get_mesh(i)] = 0;
}

ogl::element::~element()
{
    glob->free_surface(srf);
}

void ogl::element::move(const GLfloat *M, const GLfloat *I)
{
    // Cache the current transform.

    load_mat(this->M, M);
    load_mat(this->I, I);

    // Assume the VBO is bound.  Update the pretransformed vertex array.

    if (status)
        for (mesh_vert_c i = off.begin(); i != off.end(); ++i)
            (void) i->first->vert_cache(i->second, M, I);
}

//-----------------------------------------------------------------------------

GLsizei ogl::element::vcount() const
{
    // Sum all vertex counts.

    GLsizei vc = 0;

    for (mesh_vert_c i = off.begin(); i != off.end(); ++i)
        vc += i->first->vert_count();

    return vc;
}

GLsizei ogl::element::fcount() const
{
    // Sum all element counts.

    GLsizei fc = 0;

    for (mesh_vert_c i = off.begin(); i != off.end(); ++i)
        fc += i->first->face_count();

    return fc;
}

//-----------------------------------------------------------------------------

void ogl::element::enlist(mesh_elem_m& opaque,
                          mesh_elem_m& transp)
{
    // Insert each mesh into the proper opacity set.

    for (mesh_vert_c i = off.begin(); i != off.end(); ++i)
        if (i->first->state()->opaque())
            opaque.insert(mesh_elem_m::value_type(i->first, this));
        else
            transp.insert(mesh_elem_m::value_type(i->first, this));
}

ogl::vert *ogl::element::vert_cache(const ogl::mesh *m,
                                          ogl::vert *o)
{
    // Update the vertex array offset for the given mesh.

    off[m] = o;

    // Pretransform the mesh to the given vertex array.

    return m->vert_cache(o, M, I);
}

//=============================================================================
// Batch segment

ogl::segment::segment()
{
    load_idt(M);
}

ogl::segment::~segment()
{
}

void ogl::segment::move(const GLfloat *T)
{
    load_mat(M, T);
}

//-----------------------------------------------------------------------------

GLsizei ogl::segment::vcount() const
{
    // Sum all vertex counts.

    GLsizei vc = 0;

    for (element_i i = elements.begin(); i != elements.end(); ++i)
        vc += (*i)->vcount();

    return vc;
}

GLsizei ogl::segment::fcount() const
{
    // Sum all face counts.

    GLsizei fc = 0;

    for (element_i i = elements.begin(); i != elements.end(); ++i)
        fc += (*i)->fcount();

    return fc;
}

//-----------------------------------------------------------------------------

void ogl::segment::reduce(vert_p& v,
                          face_p& f, mesh_elem_m& meshes, batch_v& batches)
{
    batch_v tmp;

    // Set up the vertex and element arrays and batches for this segment.

    for (mesh_elem_i i = meshes.begin(); i != meshes.end(); ++i)
    {
        GLvoid *off = (GLvoid *) f;

        GLuint  min = std::numeric_limits<GLuint>::max();
        GLuint  max = std::numeric_limits<GLuint>::min();

        GLuint d = GLuint(v) / sizeof (vert);

        // Copy the face elements and vertices.

        v = i->second->vert_cache(i->first, v);
        f = i->first ->face_cache(f, d, min, max);

        // Create a new batch for this mesh.

        tmp.push_back(batch(i->first->state(), off,
                            i->first->face_count() * 3, min, max));
    }

    // Merge adjacent batches that share the same state binding.

    if (!tmp.empty())
    {
        batches.push_back(tmp.front());

        for (batch_i i = tmp.begin() + 1; i != tmp.end(); ++i)
            if (batches.back().bnd == i->bnd)
            {
                batches.back().num += i->num;
                batches.back().min  = std::min(i->min, batches.back().min);
                batches.back().max  = std::max(i->max, batches.back().max);
            }
            else
                batches.push_back(*i);
    }
}

void ogl::segment::enlist(vert_p& v, face_p& f)
{
    mesh_elem_m opaque;
    mesh_elem_m transp;

    // Sort all meshes.

    for (element_i i = elements.begin(); i != elements.end(); ++i)
        (*i)->enlist(opaque, transp);

    // Batch all meshes.

    opaque_bat.clear();
    transp_bat.clear();

    reduce(v, f, opaque, opaque_bat);
    reduce(v, f, transp, transp_bat);
}

void ogl::segment::draw_opaque(bool lit) const
{
    // Test the bounding box.

    // Draw all opaque meshes.

    for (batch_c i = opaque_bat.begin(); i != opaque_bat.end(); ++i)
    {
        i->bnd->bind(lit);

        glDrawRangeElementsEXT(GL_TRIANGLES, i->min, i->max, i->num,
                               GL_UNSIGNED_INT, i->off);
    }
}

void ogl::segment::draw_transp(bool lit) const
{
    // Test the bounding box.

    // Draw all transp meshes.

    for (batch_c i = transp_bat.begin(); i != transp_bat.end(); ++i)
    {
        i->bnd->bind(lit);

        glDrawRangeElementsEXT(GL_TRIANGLES, i->min, i->max, i->num,
                               GL_UNSIGNED_INT, i->off);
    }
}

//=============================================================================
// Batch manager

ogl::batcher::batcher() : vbo(0), ebo(0), status(false)
{
    glGenBuffersARB(1, &vbo);
    glGenBuffersARB(1, &ebo);
}

ogl::batcher::~batcher()
{
    if (ebo) glDeleteBuffersARB(1, &ebo);
    if (vbo) glDeleteBuffersARB(1, &vbo);
}

//-----------------------------------------------------------------------------

void ogl::batcher::dirty()
{
    status = false;
}

void ogl::batcher::clean()
{
    // Sum all vertex array and element array sizes.

    GLsizei vsz = 0;
    GLsizei fsz = 0;

    for (segment_i i = segments.begin(); i != segments.end(); ++i)
    {
        vsz += (*i)->vcount() * sizeof (vert);
        fsz += (*i)->fcount() * sizeof (face);
    }

    // Initialize vertex and element buffers of that size.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB,         vsz, 0, GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, fsz, 0, GL_STATIC_DRAW_ARB);

    // Let all segments copy to the buffers.

    vert *v = 0;
    face *f = 0;

    for (segment_i i = segments.begin(); i != segments.end(); ++i)
        (*i)->enlist(v, f);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         0);

    // Mark the buffers as clean.

    status = true;
}

//-----------------------------------------------------------------------------

void ogl::batcher::bind() const
{
    // Bind the VBO and EBO.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         vbo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
}

void ogl::batcher::free() const
{
    // Unbind the VBO and EBO.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void ogl::batcher::draw_init()
{
    // Update the VBO and EBO as necessary.

    if (!status) clean();

    // Enable and attach the vertex arrays.

    bind();

    glEnableVertexAttribArrayARB(6);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    GLsizei s = sizeof (vert);

    glTexCoordPointer       (   2, GL_FLOAT,    s, (GLubyte *) 36);
    glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, (GLubyte *) 24);
    glNormalPointer         (      GL_FLOAT,    s, (GLubyte *) 12);
    glVertexPointer         (   3, GL_FLOAT,    s, (GLubyte *)  0);

}

void ogl::batcher::draw_fini()
{
    // Disable the vertex arrays.

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableVertexAttribArrayARB(6);

    free();
}

void ogl::batcher::draw_opaque(bool lit) const
{
    // Opaque draw all segments.

    for (segment_i i = segments.begin(); i != segments.end(); ++i)
        (*i)->draw_opaque(lit);

    // TODO: something better.

    glUseProgramObjectARB(0);
}

void ogl::batcher::draw_transp(bool lit) const
{
    // Transp draw all segments.

    for (segment_i i = segments.begin(); i != segments.end(); ++i)
        (*i)->draw_transp(lit);

    // TODO: something better.

    glUseProgramObjectARB(0);
}

//=============================================================================
