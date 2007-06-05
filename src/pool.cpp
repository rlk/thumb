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

#include "glob.hpp"
#include "pool.hpp"

//=============================================================================

ogl::unit::unit(std::string name) :
    vc(0), ec(0),
    resort(true),
    rebuff(true),
    my_node(0),
    surf(glob->load_surface(name))
{
    // Create a cache mesh for each mesh of the named surface.

    for (GLsizei i = 0; i < surf->max_mesh(); ++i)
        my_mesh[new mesh] = surf->get_mesh(i);

    // Total the vertex and element counts.

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        vc += i->second->count_verts();
        ec += i->second->count_lines() * 2
            + i->second->count_faces() * 3;
    }
}

ogl::unit::~unit()
{
    // Delete all cache meshes.

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        delete i->first;

    glob->free_surface(surf);
}

void ogl::unit::set_node(node_p p)
{
    my_node = p;
}

//-----------------------------------------------------------------------------

void ogl::unit::transform(const GLfloat *M)
{
    load_mat(this->M, M);

    // Mark this unit and its node for a buffer update.

    if (my_node) my_node->set_rebuff();

    rebuff = true;
}

//-----------------------------------------------------------------------------

void ogl::unit::merge_batch(mesh_m& meshes)
{
    // Merge local meshes with the given set.  Meshes are sorted by material.

    meshes.insert(my_mesh.begin(), my_mesh.end());
}

void ogl::unit::merge_bound(aabb& b)
{
    // Merge the local mesh bounding volume with the given one.

    b.merge(my_aabb);
}

//-----------------------------------------------------------------------------

void ogl::unit::buff(GLfloat *v, GLfloat *n, GLfloat *t, GLfloat *u)
{
    if (rebuff)
    {
        my_aabb = aabb();

        // Transform and cache each mesh.  Accumulate bounding volumes.
        // TODO: maybe move this back to transfrom?

        for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        {
            i->first->cache_verts(i->second, M, I);
            i->first->merge_bound(my_aabb);
        }

        // Upload each mesh's vertex data to the bound buffer object.

        for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        {
            const GLsizei ii = i->first->count_verts();

            i->first->buffv(v, n, t, u);

            v += ii * 3;
            n += ii * 3;
            t += ii * 3;
            u += ii * 2;
        }
    }
    rebuff = false;
}
/*
void ogl::unit::sort(GLuint *e, GLuint v)
{
    // Offset and cache each mesh.

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        const GLsizei vc = i->first->count_verts();

        i->first->cache_lines(i->second, v);
        i->first->cache_faces(i->second, v);

        v += vc;
    }

    // Upload each mesh's vertex data to the bound buffer object.

    for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
    {
        const GLsizei ec = i->first->count_lines() * 2
                         + i->first->count_lines() * 3;

        i->first->buffe(e);

        e += ec;
    }
}
*/
//=============================================================================

ogl::node::node() :
    vc(0), ec(0),
    resort(true),
    rebuff(true),
    my_pool(0)
{
    load_idt(M);
}

ogl::node::~node()
{
    for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

void ogl::node::set_resort()
{
    if (my_pool) my_pool->set_resort();
    resort = true;
}

void ogl::node::set_rebuff()
{
    if (my_pool) my_pool->set_rebuff();
    rebuff = true;
}

//-----------------------------------------------------------------------------

void ogl::node::set_pool(pool_p p)
{
    my_pool = p;
}

void ogl::node::add_unit(unit_p p)
{
    // Insert the given unit into the unit set.

    my_unit.insert(p);
    p->set_node(this);

    // Include the unit's vertex and element counts.

    vc += p->vcount();
    ec += p->ecount();

    // Mark this node and its pool for a resort.

    set_resort();
}

void ogl::node::rem_unit(unit_p p)
{
    // Erase the given unit from the unit set.

    my_unit.erase(p);
    p->set_node(0);

    // Omit the unit's vertex and element counts.

    vc -= p->vcount();
    ec -= p->ecount();

    // Mark this node and its pool for a resort.

    set_resort();
}

//-----------------------------------------------------------------------------

void ogl::node::buff(GLfloat *v, GLfloat *n, GLfloat *t, GLfloat *u)
{
    // Have each unit upload its vertex data to the bound buffer objects.

    if (rebuff)
    {
        for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
        {
            GLsizei vc = (*i)->vcount();

            (*i)->buff(v, n, t, u);

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
    // Compute a minimal set of elements for the current unit set.

    if (resort)
    {
        mesh_m my_mesh;

        // Create a list of all meshes of this node, sorted by material.

        for (unit_s::iterator i = my_unit.begin(); i != my_unit.end(); ++i)
            (*i)->merge_batch(meshes);

        // Cache each offset mesh.  Upload elements to the bound buffer object.

        for (mesh_m::iterator i = my_mesh.begin(); i != my_mesh.end(); ++i)
        {
            const GLsizei vc = i->first->count_verts();
            const GLsizei ec = i->first->count_lines() * 2
                             + i->first->count_lines() * 3;

            i->first->cache_lines(i->second, v);
            i->first->cache_faces(i->second, v);

            i->first->buffe(e);

            v += vc;
            e += ec;
        }


    }
    resort = false;
}

//=============================================================================
