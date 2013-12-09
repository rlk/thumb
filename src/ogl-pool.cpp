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

#include <etc-vector.hpp>
#include <app-glob.hpp>
#include <ogl-pool.hpp>

//=============================================================================

#define get_bit(b, i) (((b) >> ((i)    )) & 1)
#define get_oct(b, i) (((b) >> ((i) * 3)) & 7)

#define set_bit(b, i, n) (((b) & (~(1 << ((i)    )))) | ((n) << ((i)    )))
#define set_oct(b, i, n) (((b) & (~(7 << ((i) * 3)))) | ((n) << ((i) * 3)))

//-----------------------------------------------------------------------------

ogl::elem::elem(const binding *b,
                const GLuint  *o, GLenum t, GLsizei n, GLuint a, GLuint z) :
    bnd(b),
    off(o),
    typ(t),
    num(n),
    min(a),
    max(z)
{
}

bool ogl::elem::depth_eq(const elem& that) const
{
    // Determine whether that element batch may be depth-mode merged with this.

    if (typ == that.typ && off + num == that.off)
    {
        if (bnd && that.bnd) return bnd->depth_eq(that.bnd);
    }
    return false;
}

bool ogl::elem::color_eq(const elem& that) const
{
    // Determine whether that element batch may be color-mode merged with this.

    if (typ == that.typ && off + num == that.off)
    {
        if (bnd && that.bnd) return bnd->color_eq(that.bnd);
    }
    return false;
}

void ogl::elem::merge(const elem& that)
{
    // Merge that element batch with this.

    num += that.num;
    min  = std::min(min, that.min);
    max  = std::max(max, that.max);
}

void ogl::elem::draw(bool color) const
{
    // Bind this batch's state and render all elements.

    if (bnd)
        bnd->bind(color);

    glDrawRangeElements(typ, min, max, num, GL_UNSIGNED_INT, off);
}

//=============================================================================

ogl::unit::unit(std::string name, bool center) :
    vc(0),
    ec(0),
    my_node(0),
    rebuff(true),
    active(true),
    surf(glob->load_surface(name, center))
{
    set_mesh();
}

ogl::unit::unit(const unit& that) :
    vc(0),
    ec(0),
    my_node(0),
    rebuff(true),
    active(true),
    surf(glob->dupe_surface(that.surf))
{
    M = that.M;
    I = that.I;

    set_mesh();
}

ogl::unit::~unit()
{
    // Delete all cache meshes.

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        delete i->second;

    glob->free_surface(surf);
}

//-----------------------------------------------------------------------------

// The following rendering functions are NOT on the primary display path. They
// are used only to cherry-pick units for transient highlighting. Optimized
// rendering is handled by the batch node.

void ogl::unit::draw_lines() const
{
    for (mesh_m::const_iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        i->first->state()->bind(true);
        i->second->draw_lines();
    }
}

void ogl::unit::draw_faces() const
{
    for (mesh_m::const_iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        i->first->state()->bind(true);
        i->second->draw_faces();
    }
}

//-----------------------------------------------------------------------------

void ogl::unit::set_mesh()
{
    // Create a cache for each mesh.  Count vertices and elements.

    for (size_t i = 0; surf && i < surf->max_mesh(); ++i)
    {
        const mesh *m = surf->get_mesh(i);

        my_mesh.insert(mesh_m::value_type(m, new mesh));

        vc += m->count_verts();
        ec += m->count_lines() * 2
            + m->count_faces() * 3;

        m->merge_bound(my_aabb);
    }
}

void ogl::unit::set_node(node_p p)
{
    my_node = p;
}

void ogl::unit::set_mode(bool b)
{
    if (my_node) my_node->set_resort();
    active = b;
}

//-----------------------------------------------------------------------------

void ogl::unit::transform(const mat4& M, const mat4& I)
{
    this->M = M;
    this->I = I;

    // Mark this unit and its node for a buffer update.

    if (my_node) my_node->set_rebuff();
    rebuff = true;
}

//-----------------------------------------------------------------------------

void ogl::unit::merge_batch(mesh_m& meshes)
{
    // Merge local meshes with the given set.  Meshes are sorted by material.

    if (active) meshes.insert(my_mesh.begin(), my_mesh.end());
}

void ogl::unit::merge_bound(aabb& b)
{
    // Merge the local mesh bounding volume with the given one.

    if (active) b.merge(my_aabb);
}

//-----------------------------------------------------------------------------

void ogl::unit::buff(bool b)
{
    if (b || rebuff)
    {
        my_aabb = aabb();

        // Transform and cache each mesh.  Accumulate bounding volumes.

        for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        {
            i->second->cache_verts(i->first, M, I);
            i->second->merge_bound(my_aabb);
        }
    }
    rebuff = false;
}

//=============================================================================

ogl::node::node() :
    vc(0), ec(0),
    rebuff(true),
    my_pool(0),
    test_cache(0xFFFFFFFF),
    hint_cache(0x00000000)
{
}

ogl::node::~node()
{
    for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

void ogl::node::clear()
{
    while (!my_unit.empty())
        rem_unit(*my_unit.begin());
}

void ogl::node::set_rebuff()
{
    if (my_pool) my_pool->set_rebuff();
    rebuff = true;
}

void ogl::node::set_resort()
{
    if (my_pool) my_pool->set_resort();
}

//-----------------------------------------------------------------------------

void ogl::node::set_pool(pool_p p)
{
    my_pool = p;
}

void ogl::node::add_unit(unit_p p)
{
    if (p && my_unit.find(p) == my_unit.end())
    {
        // Insert the given unit into the unit set.

        my_unit.insert(p);
        p->set_node(this);

        // Include the unit's vertex and element counts.

        vc += p->vcount();
        ec += p->ecount();

        if (my_pool) my_pool->add_vcount(+p->vcount());
        if (my_pool) my_pool->add_ecount(+p->ecount());

        // Mark this node's pool for a resort.

        if (my_pool) my_pool->set_resort();
    }
}

void ogl::node::rem_unit(unit_p p)
{
    if (p && my_unit.find(p) != my_unit.end())
    {
        // Erase the given unit from the unit set.

        my_unit.erase(p);
        p->set_node(0);

        // Omit the unit's vertex and element counts.

        vc -= p->vcount();
        ec -= p->ecount();

        if (my_pool) my_pool->add_vcount(-p->vcount());
        if (my_pool) my_pool->add_ecount(-p->ecount());

        // Mark this node's pool for a resort.

        if (my_pool) my_pool->set_resort();
    }
}

//-----------------------------------------------------------------------------

void ogl::node::buff(GLfloat *v, GLfloat *n, GLfloat *t, GLfloat *u, bool b)
{
    if (b || rebuff)
    {
        // Have each unit pretransform its vertex data and compute its bound.

        my_aabb = aabb();

        for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
        {
            (*i)->buff(b);
            (*i)->merge_bound(my_aabb);
        }

        // Upload each mesh's vertex data to the bound buffer object.

        for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        {
            const GLsizei vc = i->second->count_verts();

            i->second->buffv(v, n, t, u);

            v += vc * 3;
            n += vc * 3;
            t += vc * 3;
            u += vc * 2;
        }
    }
    rebuff = false;
}

void ogl::node::sort(GLuint *e, GLuint d)
{
    // Create a list of all meshes of this node, sorted by material.

    my_mesh.clear();

    for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
        (*i)->merge_batch(my_mesh);

    // Create a list of all element batches of this node.

    elem_v my_elem;

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        const GLsizei dc = i->first->count_verts();
        const GLsizei fc = i->first->count_faces() * 3;
        const GLsizei lc = i->first->count_lines() * 2;

        // Cache each offset mesh.

        i->second->cache_faces(i->first, d);
        i->second->cache_lines(i->first, d);

        // Upload elements to the bound buffer object.

        i->second->buffe(e);

        // Create a batch for each set of primatives.

        if (fc) my_elem.push_back(elem(i->first->state(), e, GL_TRIANGLES, fc,
                                       i->second->get_min(),
                                       i->second->get_max()));
        e += fc;

        if (lc) my_elem.push_back(elem(i->first->state(), e, GL_LINES,     lc,
                                       i->second->get_min(),
                                       i->second->get_max()));
        e += lc;
        d += dc;
    }

    // Create a minimal vector of batches for each draw mode.

    opaque_depth.clear();
    opaque_color.clear();
    masked_depth.clear();
    masked_color.clear();

    for (elem_v::iterator i = my_elem.begin(); i != my_elem.end(); ++i)
    {
        if (i->opaque())
        {
            // Opaque depth batches

            if (opaque_depth.empty() || !opaque_depth.back().depth_eq(*i))
                opaque_depth.push_back(*i);
            else
                opaque_depth.back().merge(*i);

            // Opaque color batches

            if (opaque_color.empty() || !opaque_color.back().color_eq(*i))
                opaque_color.push_back(*i);
            else
                opaque_color.back().merge(*i);
        }
        else
        {
            // Masked depth batches

            if (masked_depth.empty() || !masked_depth.back().depth_eq(*i))
                masked_depth.push_back(*i);
            else
                masked_depth.back().merge(*i);

            // Masked color batches

            if (masked_color.empty() || !masked_color.back().color_eq(*i))
                masked_color.push_back(*i);
            else
                masked_color.back().merge(*i);
        }
    }
}

//-----------------------------------------------------------------------------

void ogl::node::transform(const mat4& M)
{
    this->M = M;
}

//-----------------------------------------------------------------------------

ogl::aabb ogl::node::view(int id, const vec4 *V, int n)
{
    // Get the cached culler hint.

    int bit, hint = get_oct(hint_cache, id);

    // Test the bounding box and set the visibility bit.

    if (V == 0 || my_aabb.test(V, n, M, hint))
        test_cache = set_bit(test_cache, id, (bit = 1));
    else
        test_cache = set_bit(test_cache, id, (bit = 0));

    // Set the cached culler hint.

    hint_cache = set_oct(hint_cache, id, hint);

    // If this node is visible, return the world-space AABB.

    if (bit && V)
        return ogl::aabb(my_aabb, M);
    else
        return ogl::aabb();
}

void ogl::node::draw(int id, bool color, bool alpha)
{
    // Proceed if this node passed visibility test ID.

    if (get_bit(test_cache, id))
    {
        // Select the batch vector.  Confirm that it is non-empty.

        elem_i b;
        elem_i e;

        if (color)
            if (alpha) { b = masked_color.begin(); e = masked_color.end(); }
            else       { b = opaque_color.begin(); e = opaque_color.end(); }
        else
            if (alpha) { b = masked_depth.begin(); e = masked_depth.end(); }
            else       { b = opaque_depth.begin(); e = opaque_depth.end(); }

        if (b != e)
        {
            if (alpha) { glEnable(GL_ALPHA_TEST); };

            // Render the selected batches.

            glPushMatrix();
            {
                glMultMatrixd(transpose(M));

                for (elem_i i = b; i != e; ++i) i->draw(color);

//              TODO: make this an option  ogl::draw_bounding_boxes
//              my_aabb.draw();
            }
            glPopMatrix();

            if (alpha) { glDisable(GL_ALPHA_TEST); };
        }
    }
}

//=============================================================================

ogl::pool::pool() : vc(0), ec(0), resort(true), rebuff(true), vbo(0), ebo(0)
{
    init();
}

ogl::pool::~pool()
{
    for (node_s::iterator i = my_node.begin(); i != my_node.end(); ++i)
        delete (*i);

    fini();
}

//-----------------------------------------------------------------------------

void ogl::pool::set_resort()
{
    resort = true;
}

void ogl::pool::set_rebuff()
{
    rebuff = true;
}

void ogl::pool::add_vcount(GLsizei vc)
{
    this->vc += vc;
}

void ogl::pool::add_ecount(GLsizei ec)
{
    this->ec += ec;
}

//-----------------------------------------------------------------------------

void ogl::pool::add_node(node_p p)
{
    // Insert the given node into the node set.

    my_node.insert(p);
    p->set_pool(this);

    // Include the node's vertex and element counts.

    vc += p->vcount();
    ec += p->ecount();

    // Mark this pool and its pool for a resort.

    set_resort();
}

void ogl::pool::rem_node(node_p p)
{
    // Erase the given node from the node set.

    my_node.erase(p);
    p->set_pool(0);

    // Omit the node's vertex and element counts.

    vc -= p->vcount();
    ec -= p->ecount();

    // Mark this pool and its pool for a resort.

    set_resort();
}

//-----------------------------------------------------------------------------

void ogl::pool::buff(bool force)
{
    // Compute buffer object offsets for each vertex attribute.

    GLfloat *v = (GLfloat *) (0);
    GLfloat *n = (GLfloat *) (vc * sizeof (GLfloat) * 3);
    GLfloat *t = (GLfloat *) (vc * sizeof (GLfloat) * 6);
    GLfloat *u = (GLfloat *) (vc * sizeof (GLfloat) * 9);

    // Rebuff all nodes.

    for (node_s::iterator i = my_node.begin(); i != my_node.end(); ++i)
    {
        const GLsizei vc = (*i)->vcount();

        (*i)->buff(v, n, t, u, force);

        v += vc * 3;
        n += vc * 3;
        t += vc * 3;
        u += vc * 2;
    }
    rebuff = false;
}

void ogl::pool::sort()
{
    GLsizei vsz = vc * sizeof (GLfloat) * 11;
    GLsizei esz = ec * sizeof (GLuint);

    // Initialize vertex and element buffer sizes.

    glBufferData(GL_ARRAY_BUFFER,         vsz, 0, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, esz, 0, GL_STATIC_DRAW);

    // Resort all nodes.

    GLuint *e = 0;
    GLuint  d = 0;

    for (node_s::iterator i = my_node.begin(); i != my_node.end(); ++i)
    {
        (*i)->sort(e, d);

        e += (*i)->ecount();
        d += (*i)->vcount();
    }
    resort = false;

    // Force-rebuff all nodes.

    buff(true);
}

//-----------------------------------------------------------------------------

void ogl::pool::prep()
{
    // Bind the VBO and EBO.

    if (resort || rebuff)
    {
        glBindBuffer(GL_ARRAY_BUFFER,         vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }

    // Update the VBO and EBO as necessary.

    if (resort) sort(     );
    if (rebuff) buff(false);

    // Unbind the VBO and EBO.

    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ogl::aabb ogl::pool::view(int id, const vec4 *V, int n)
{
    ogl::aabb b;

    // Test all nodes for visibility. Find the union of their bounds.

    for (node_s::iterator i = my_node.begin(); i != my_node.end(); ++i)
        b.merge((*i)->view(id, V, n));

    return b;
}

//-----------------------------------------------------------------------------

void ogl::pool::draw_init()
{
    // Bind the VBO and EBO.

    glBindBuffer(GL_ARRAY_BUFFER,         vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Enable and attach the vertex arrays.

    glEnableVertexAttribArray(6);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    GLfloat *v = (GLfloat *) (0);
    GLfloat *n = (GLfloat *) (vc * sizeof (GLfloat) * 3);
    GLfloat *t = (GLfloat *) (vc * sizeof (GLfloat) * 6);
    GLfloat *u = (GLfloat *) (vc * sizeof (GLfloat) * 9);

    glTexCoordPointer    (   2, GL_FLOAT,    sizeof (GLvec2), u);
    glVertexAttribPointer(6, 3, GL_FLOAT, 0, sizeof (GLvec3), t);
    glNormalPointer      (      GL_FLOAT,    sizeof (GLvec3), n);
    glVertexPointer      (   3, GL_FLOAT,    sizeof (GLvec3), v);
}

void ogl::pool::draw(int id, bool color, bool alpha)
{
    // Draw all nodes.

    for (node_s::iterator i = my_node.begin(); i != my_node.end(); ++i)
        (*i)->draw(id, color, alpha);
}

void ogl::pool::draw_fini()
{
    // Disable the vertex arrays.

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableVertexAttribArray(6);

    // Unbind the VBO and EBO.

    glBindBuffer(GL_ARRAY_BUFFER,         0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//-----------------------------------------------------------------------------

void ogl::pool::init()
{
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    resort = true;
    rebuff = true;
}

void ogl::pool::fini()
{
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);

    ebo = 0;
    vbo = 0;
}

//=============================================================================
