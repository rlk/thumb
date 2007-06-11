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

#ifndef POOL_HPP
#define POOL_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

#include "surface.hpp"
#include "mesh.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    //-------------------------------------------------------------------------

    class unit;
    class node;
    class pool;

    typedef unit                      *unit_p;
    typedef std::set<unit_p>           unit_s;
    typedef std::set<unit_p>::iterator unit_i;

    typedef node                      *node_p;
    typedef std::set<node_p>           node_s;
    typedef std::set<node_p>::iterator node_i;

    typedef pool                      *pool_p;
    typedef std::set<pool_p>           pool_s;
    typedef std::set<pool_p>::iterator pool_i;

    typedef std::multimap<const mesh *, mesh_p, meshcmp> mesh_m;

    //-------------------------------------------------------------------------
    // Drawable / mergable element batch

    class elem
    {
        const binding *bnd;
        const GLuint  *off;

        GLenum  typ;
        GLsizei num;
        GLuint  min;
        GLuint  max;

    public:

        elem(const binding *, const GLuint *, GLenum, GLsizei, GLuint, GLuint);

        bool opaque() const { return bnd->opaque(); }

        bool depth_eq(const elem&) const;
        bool color_eq(const elem&) const;
        void merge   (const elem&);

        void draw(bool) const;
    };

    // TODO: deque?

    typedef std::vector<elem>                 elem_v;
    typedef std::vector<elem>::const_iterator elem_i;

    //-------------------------------------------------------------------------
    // Static batchable

    class unit
    {
        GLfloat M[16];
        GLfloat I[16];

        GLsizei vc;
        GLsizei ec;

        node_p my_node;
        mesh_m my_mesh;
        aabb   my_aabb;

        bool rebuff;

        const surface *surf;

        void set_mesh();

    public:

        unit(std::string);
        unit(const unit&);
       ~unit();

        void set_node(node_p);

        void transform(const GLfloat *, const GLfloat *);

        void merge_batch(mesh_m&);
        void merge_bound(aabb&);

        void buff(bool);

        GLsizei vcount() const { return vc; }
        GLsizei ecount() const { return ec; }
    };

    //-------------------------------------------------------------------------
    // Dynamic batchable

    class node
    {
        GLfloat M[16];

        GLsizei vc;
        GLsizei ec;

        bool rebuff;

        pool_p my_pool;
        unit_s my_unit;
        mesh_m my_mesh;
        aabb   my_aabb;

        elem_v opaque_depth;
        elem_v opaque_color;
        elem_v transp_depth;
        elem_v transp_color;

    public:

        node();
       ~node();

        void clear();

        void set_rebuff();

        void set_pool(pool_p);
        void add_unit(unit_p);
        void rem_unit(unit_p);

        void buff(bool, GLfloat *, GLfloat *, GLfloat *, GLfloat *);
        void sort(GLuint  *, GLuint);

        void draw(bool, bool);

        GLsizei vcount() const { return vc; }
        GLsizei ecount() const { return ec; }

        void transform(const GLfloat *);
    };

    //-------------------------------------------------------------------------
    // Batch pool

    class pool
    {
        GLsizei vc;
        GLsizei ec;

        bool resort;
        bool rebuff;

        GLuint vbo;
        GLuint ebo;

        node_s my_node;

        void buff(bool);
        void sort();

    public:

        pool();
       ~pool();

        void set_resort();
        void set_rebuff();
        void add_vcount(GLsizei);
        void add_ecount(GLsizei);

        void add_node(node_p);
        void rem_node(node_p);

        void draw_init();
        void draw_fini();

        void draw(bool, bool);

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
