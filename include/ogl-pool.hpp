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

#ifndef OGL_POOL_HPP
#define OGL_POOL_HPP

#include <string>
#include <vector>
#include <set>
#include <map>

#include <etc-vector.hpp>
#include <ogl-surface.hpp>
#include <ogl-mesh.hpp>

// This interface, in consort with ogl::mesh, implements a fairly complex
// mechanism to optimize 3D geometry for rendering with OpenGL vertex buffer
// objects. It provides a top-level ogl::pool object containing a two-level
// scene graph. First-level graph entities, ogl::node objects, represent
// transformable accumulations of static second-level entities, ogl::unit
// objects.

// Each unit maintains two sets of meshes, one set of static meshes given by its
// input OBJ file, and a second set of cached meshes giving transformed versions
// of that input. These transformed vertex arrays are concatenated giving vertex
// array buffers. The meshes are sorted by material type and concatenated giving
// element array buffers.

// Material definitions, given by ogl::surface objects, define independent color
// and depth bindings. Meshes are sorted accordingly, giving a separate set of
// element array blocks for depth-only rendering, and thus optimizing shadow map
// and early-Z passes. Alpha-tested geometry is further distinguised, allowing
// alpha-test geometry to be rendered last.

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
    public:

        elem(const binding *, const GLuint *, GLenum, GLsizei, GLuint, GLuint);

        bool opaque() const { return bnd ? bnd->opaque() : true; }

        bool depth_eq(const elem&) const;
        bool color_eq(const elem&) const;
        void merge   (const elem&);

        void draw(bool) const;

    private:

        const binding *bnd;
        const GLuint  *off;

        GLenum  typ;
        GLsizei num;
        GLuint  min;
        GLuint  max;
    };

    // TODO: deque?

    typedef std::vector<elem>                 elem_v;
    typedef std::vector<elem>::const_iterator elem_i;

    //-------------------------------------------------------------------------
    // Static batchable

    class unit
    {
    public:

        unit(std::string, bool=true);
        unit(const unit&);
       ~unit();

        void draw_lines() const;
        void draw_faces() const;

        void set_node(node_p);
        void set_mode(bool);
        void set_ubiq(bool);

        bool is_ubiq() const { return ubiquitous; }

        void transform(const mat4&, const mat4&);

        void merge_batch(mesh_m&);

        void buff(bool);

        GLsizei vcount() const { return vc; }
        GLsizei ecount() const { return ec; }
        int     get_id() const { return id; }
        aabb get_bound() const { return my_aabb; }

        mat4                get_world_transform() const;
        const ogl::binding *get_default_binding() const;

    private:

        static int serial;

        mat4 M;
        mat4 I;
        int id;

        GLsizei vc;
        GLsizei ec;

        node_p my_node;
        mesh_m my_mesh;
        aabb   my_aabb;

        bool rebuff;
        bool active;
        bool ubiquitous;

        const surface *surf;

        void set_mesh();
    };

    //-------------------------------------------------------------------------
    // Dynamic batchable

    class node
    {
    public:

        node();
       ~node();

        void clear();

        void set_rebuff();
        void set_resort();

        void set_pool(pool_p);
        void add_unit(unit_p);
        void rem_unit(unit_p);

        void buff(GLfloat *, GLfloat *, GLfloat *, GLfloat *, bool);
        void sort(GLuint  *, GLuint);

        ogl::aabb view(int, const vec4 *, int);
        void      draw(int=0, bool=true, bool=false);

        GLsizei vcount() const { return vc; }
        GLsizei ecount() const { return ec; }

        void transform(const mat4&);

        mat4 get_world_transform() const;

    private:

        mat4 M;

        GLsizei vc;
        GLsizei ec;

        bool ubiquitous;
        bool rebuff;

        pool_p my_pool;
        unit_s my_unit;
        mesh_m my_mesh;
        aabb   my_aabb;

        unsigned int test_cache;
        unsigned int hint_cache;

        elem_v opaque_depth;
        elem_v opaque_color;
        elem_v masked_depth;
        elem_v masked_color;
    };

    //-------------------------------------------------------------------------
    // Batch pool

    class pool
    {
    public:

        pool();
       ~pool();

        void set_resort();
        void set_rebuff();
        void add_vcount(GLsizei);
        void add_ecount(GLsizei);

        void add_node(node_p);
        void rem_node(node_p);

        ogl::aabb view(int, const vec4 *, int);
        void      prep();

        void draw_init();
        void draw(int=0, bool=true, bool=false);
        void draw_fini();

        void init();
        void fini();

    private:

        GLsizei vc;
        GLsizei ec;

        bool resort;
        bool rebuff;

        GLuint vbo;
        GLuint ebo;

        node_s my_node;

        void buff(bool);
        void sort();
    };
}

//-----------------------------------------------------------------------------

#endif
