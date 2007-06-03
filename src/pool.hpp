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

    typedef std::map<mesh *, const mesh *> mesh_m;

    //-------------------------------------------------------------------------

    enum alpha_mode { OPAQUE,  TRANSP };
    enum light_mode { ZBUFFER, AMBIENT, DIFFUSE };

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

        void merge(elem&);
        void draw() const;
    };

    typedef std::vector<elem>           elem_v;
    typedef std::vector<elem>::iterator elem_i;

    //-------------------------------------------------------------------------
    // Static batchable

    class unit
    {
        GLfloat M[16];

        GLfloat bound_min[3];
        GLfloat bound_max[3];
        GLfloat bound_rad[1];

        bool resort;
        bool rebuff;

        node_p my_node;
        mesh_m my_mesh;

    public:

        unit(std::string);
       ~unit();

        void set_node(node_p);

        void transform(const GLfloat *);

        GLsizei vcount() const;
        GLsizei ecount() const;

        void merge_meshes(mesh_s&);
        void merge_bounds(GLfloat[3], GLfloat[3], GLfloat[1]);
    };

    //-------------------------------------------------------------------------
    // Dynamic batchable

    class node
    {
        GLfloat M[16];

        GLfloat bound_min[3];
        GLfloat bound_max[3];
        GLfloat bound_rad[1];

        bool resort;
        bool rebuff;

        pool_p my_pool;
        unit_s my_unit;
        elem_v my_elem[2][3];

    public:

        node();
       ~node();

        void set_resort();
        void set_rebuff();

        void set_pool(pool_p);
        void add_unit(unit_p);
        void rem_unit(unit_p);

        GLsizei vcount() const;
        GLsizei ecount() const;

        void buff(GLuint *, GLfloat *, GLfloat *, GLfloat *, GLfloat *);

        void draw(enum alpha_mode, enum light_mode);
    };

    //-------------------------------------------------------------------------
    // Batch pool

    class pool
    {
        bool resort;
        bool rebuff;

        GLuint vbo;
        GLuint nbo;
        GLuint tbo;
        GLuint ubo;
        GLuint ebo;

        node_s my_node;

    public:

        pool();
       ~pool();

        void set_resort();
        void set_rebuff();

        void add_node(node_p);
        void rem_node(node_p);

        void draw(enum alpha_mode, enum light_mode);
    };
}

//-----------------------------------------------------------------------------

#endif
