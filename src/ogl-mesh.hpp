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

#ifndef MESH_HPP
#define MESH_HPP

#include <vector>

#include "ogl-opengl.hpp"
#include "ogl-aabb.hpp"
#include "ogl-binding.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    //-------------------------------------------------------------------------

    struct vec2
    {
        GLfloat v[2];

        vec2() { v[0] = v[1] = 0.0f; }
    };

    struct vec3
    {
        GLfloat v[3];

        vec3() { v[0] = v[1] = v[2] = 0.0f; }
    };

    typedef std::vector<vec2> vec2_v;
    typedef std::vector<vec3> vec3_v;

    //-------------------------------------------------------------------------

    struct face
    {
        GLuint i;
        GLuint j;
        GLuint k;

        face()                                                { }
        face(GLuint I, GLuint J, GLuint K) : i(I), j(J), k(K) { }
    };

    typedef face                             *face_p;
    typedef std::vector<face>                 face_v;
    typedef std::vector<face>::iterator       face_i;
    typedef std::vector<face>::const_iterator face_c;

    //-------------------------------------------------------------------------

    struct line
    {
        GLuint i;
        GLuint j;

        line()                                { }
        line(GLuint I, GLuint J) : i(I), j(J) { }
    };

    typedef line                             *line_p;
    typedef std::vector<line>                 line_v;
    typedef std::vector<line>::iterator       line_i;
    typedef std::vector<line>::const_iterator line_c;

    //-------------------------------------------------------------------------

    class mesh
    {
        const binding *material;

        // Vertex buffers

        vec3_v vv;
        vec3_v nv;
        vec3_v tv;
        vec2_v uv;

        // Element buffers

        face_v faces;
        line_v lines;

        // Vertex bound and element range

        aabb bound;
        GLuint min;
        GLuint max;

        // Buffer object cache state

        bool dirty_verts;
        bool dirty_faces;
        bool dirty_lines;

    public:

        mesh(std::string&);
        mesh();
       ~mesh();

        // State modifiers

        void calc_tangent();

        void add_vert(vec3&,  vec3&,  vec2&);
        void add_face(GLuint, GLuint, GLuint);
        void add_line(GLuint, GLuint);
        
        // Cache modifiers

        void cache_verts(const mesh *, const double *, const double *);
        void cache_faces(const mesh *, GLuint);
        void cache_lines(const mesh *, GLuint);

        // Accessors

        const binding *state() const { return material; }

        GLsizei count_verts() const { return    vv.size(); }
        GLsizei count_faces() const { return faces.size(); }
        GLsizei count_lines() const { return lines.size(); }

        void merge_bound(aabb& b) const { b.merge(bound); }

        GLuint get_min() const { return min; }
        GLuint get_max() const { return max; }

        // Buffer object writers

        void buffv(GLfloat *, GLfloat *, GLfloat *, GLfloat *);
        void buffe(GLuint  *);
    };

    typedef mesh                               *mesh_p;
    typedef std::vector<mesh *>                 mesh_v;
    typedef std::vector<mesh *>::iterator       mesh_i;
    typedef std::vector<mesh *>::const_iterator mesh_c;

    //-------------------------------------------------------------------------

    struct meshcmp
    {
        bool operator()(const mesh *m1, const mesh *m2) const {
            return (m1->count_verts() > m2->count_verts());
        }
    };
}

//-----------------------------------------------------------------------------

#endif
