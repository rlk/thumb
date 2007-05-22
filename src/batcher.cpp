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

//-----------------------------------------------------------------------------
/*
ogl::batch::batch(std::string name) :
    bnd(glob->load_binding(name)),
    vptr(0), vlen(0),
    eptr(0), elen(0),
    emin(0), emax(0)
{
}

ogl::batch::batch(batch const& that)
{
   *this = that;
    glob->dupe_binding(bnd);
}

ogl::batch::~batch()
{
    glob->free_binding(bnd);
}

ogl::batch& ogl::batch::operator=(batch const& that)
{
   *this = that;
    glob->dupe_binding(bnd);

    return *this;
}

bool ogl::batch::operator<(batch const& that)
{
    return (bnd < that.bnd);
}

bool ogl::batch::merge(batch const& that)
{
    if (bnd == that.bnd)
    {
        vlen += that.vlen;
        elen += that.elen;

        emin = std::min(emin, that.emin);
        emax = std::max(emin, that.emax);

        return true;
    }
    return false;
}

void ogl::batch::draw() const
{
    bnd->bind();

    glDrawRangeElements(GL_TRIANGLES, emin, emax, elen / sizeof (GLuint), eptr);
}
*/

//-----------------------------------------------------------------------------

ogl::element::element(bool& dirty, std::string name) :
    dirty(dirty), srf(glob->load_surface(name))
{
    dirty = true;

    // Clone all meshes.
}

ogl::element::~element()
{
    glob->free_surface(srf);
}

void ogl::element::move(const GLfloat *T)
{
    // If the vbo is dirty just update the matrix
    // else pretransform all batches
}

GLsizei ogl::element::vcount() const
{
    // Sum all vertex array array sizes.
    return 0;
}

GLsizei ogl::element::icount() const
{
    // Sum all element array sizes.
    return 0;
}

void ogl::element::enlist(mesh_set& opaque,
                          mesh_set& transp)
{
    // Add mesh pointers to sets based on opacity.
}

//-----------------------------------------------------------------------------

ogl::segment::segment(bool& dirty) : dirty(dirty)
{
    dirty = true;
}

ogl::segment::~segment()
{
    for (element_set::iterator i = elements.begin(); i != elements.end(); ++i)
        delete (*i);
}

ogl::element *ogl::segment::add(std::string name)
{
    element *elt = new element(dirty, name);
    elements.insert(elt);
    return elt;
}

void ogl::segment::del(ogl::element *elt)
{
    elements.erase(elt);
    delete elt;
}

void ogl::segment::move(const GLfloat *T)
{
    load_mat(M, T);
}

GLsizei ogl::segment::vsize() const
{
    // Sum all vertex array array sizes.

    GLsizei vsz = 0;

    for (element_set::iterator i = elements.begin(); i != elements.end(); ++i)
        vsz += (*i)->vsize();

    return vsz;
}

GLsizei ogl::segment::esize() const
{
    // Sum all element array sizes.

    GLsizei esz = 0;

    for (element_set::iterator i = elements.begin(); i != elements.end(); ++i)
        esz += (*i)->esize();

    return esz;
}

void ogl::segment::clean(GLvoid *vptr, GLvoid *voff,
                         GLvoid *eptr, GLvoid *eoff)
{
    // Combine all batches to a vector
    // Cache all batch data
    // Merge adjacent batches with eqv binding
}

void ogl::segment::draw_opaque(bool lit) const
{
    // test the bounding box
    // iterate over the opaque set
    //     accumulate all batches with current state
    //         draw
}

void ogl::segment::draw_transp(bool lit) const
{
    // test the bounding box
    // iterate over the transp set
    //     accumulate all batches with current state
    //         draw
}

//-----------------------------------------------------------------------------

ogl::batcher::batcher() : vbo(0), ebo(0), dirty(true)
{
    glGenBuffersARB(1, &vbo);
    glGenBuffersARB(1, &ebo);
}

ogl::batcher::~batcher()
{
    for (segment_set::iterator i = segments.begin(); i != segments.end(); ++i)
        delete (*i);
    
    if (ebo) glDeleteBuffersARB(1, &ebo);
    if (vbo) glDeleteBuffersARB(1, &vbo);
}

ogl::segment *ogl::batcher::add()
{
    segment *seg = new segment(dirty);
    segments.insert(seg);
    return seg;
}

void ogl::batcher::del(ogl::segment *seg)
{
    segments.erase(seg);
    delete seg;
}

void ogl::batcher::clean()
{
    // Sum all vertex array and element array sizes.

    GLsizei vsz = 0;
    GLsizei esz = 0;

    for (segment_set::iterator i = segments.begin(); i != segments.end(); ++i)
    {
        vsz += (*i)->vsize();
        esz += (*i)->esize();
    }

    // Initialize vertex and element buffers of that size.

    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB,         vsz, 0, GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, esz, 0, GL_STATIC_DRAW_ARB);

    // Let all segments copy to the buffers.

    GLubyte *e;
    GLubyte *v;

    e = (GLubyte *) glMapBufferARB(GL_ARRAY_BUFFER_ARB,         GL_WRITE_ONLY);
    v = (GLubyte *) glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

    vsz = 0;
    esz = 0;

    for (segment_set::iterator i = segments.begin(); i != segments.end(); ++i)
    {
        (*i)->clean(v + vsz, (GLvoid *) vsz,
                    e + esz, (GLvoid *) esz);

        vsz += (*i)->vsize();
        esz += (*i)->esize();
    }

    glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
    glBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);

    // Mark the buffers as clean.

    dirty = false;
}

void ogl::batcher::draw_init()
{
    // Update the VBO and EBO as necessary.

    if (dirty) clean();

    // Bind the VBO and EBO.

    GLsizei s = sizeof (ogl::vert);

    glEnableVertexAttribArrayARB(6);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    glTexCoordPointer       (   2, GL_FLOAT,    s, (GLubyte *) 36);
    glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, (GLubyte *) 24);
    glNormalPointer         (      GL_FLOAT,    s, (GLubyte *) 12);
    glVertexPointer         (   3, GL_FLOAT,    s, (GLubyte *)  0);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ebo);
}

void ogl::batcher::draw_fini()
{
    // Unbind the VBO and EBO.

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB,         0);
}

void ogl::batcher::draw_opaque(bool lit) const
{
    // Opaque draw all segments.

    for (segment_set::iterator i = segments.begin(); i != segments.end(); ++i)
        (*i)->draw_opaque(lit);
}

void ogl::batcher::draw_transp(bool lit) const
{
    // Transp draw all segments.

    for (segment_set::iterator i = segments.begin(); i != segments.end(); ++i)
        (*i)->draw_transp(lit);
}

//-----------------------------------------------------------------------------
