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

#include "opengl.hpp"
#include "binding.hpp"

//-----------------------------------------------------------------------------

namespace ogl
{
    class GLfloat2
    {
        GLfloat v[2];

    public:

        GLfloat& operator[](int i) { return v[i]; }
    };

    class GLfloat3
    {
        GLfloat v[3];

    public:

        GLfloat& operator[](int i) { return v[i]; }
    };

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

    struct vert
    {
        vec3    v;
        vec3    n;
        vec3    t;
        vec2    s;

        vert() { }

        vert(vec3_v& vv, vec2_v& sv, vec3_v& nv, int vi, int si, int ni) {
            v = (vi >= 0) ? vv[vi] : vec3();
            s = (si >= 0) ? sv[si] : vec2();
            n = (ni >= 0) ? nv[ni] : vec3();
            t =                      vec3();
        }
    };

    typedef vert                             *vert_p;
    typedef std::vector<vert>                 vert_v;
    typedef std::vector<vert>::iterator       vert_i;
    typedef std::vector<vert>::const_iterator vert_c;

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

        vert_v verts;
        face_v faces;
        line_v lines;

    public:

        mesh(std::string&);
        mesh();
       ~mesh();

        void calc_tangent();

        // State handlers

        const binding *state() const { return material; }

        void    add_vert(vert v)   { verts.push_back(v); }
        void    add_face(face f)   { faces.push_back(f); }
        void    add_line(line l)   { lines.push_back(l); }

        GLsizei vert_count() const { return verts.size(); }
        GLsizei face_count() const { return faces.size(); }
        GLsizei line_count() const { return lines.size(); }

        // Batch initializers

        vert_p  vert_cache(vert_p, const GLfloat *, const GLfloat *) const;
        GLuint *face_cache(GLuint *, GLuint, GLuint&, GLuint&)       const;
        GLuint *line_cache(GLuint *, GLuint, GLuint&, GLuint&)       const;

        // Bound computers

        void box_bound(GLfloat *) const;
        void sph_bound(GLfloat *) const;
    };

    typedef mesh                               *mesh_p;
    typedef std::vector<mesh *>                 mesh_v;
    typedef std::vector<mesh *>::iterator       mesh_i;
    typedef std::vector<mesh *>::const_iterator mesh_c;
}


//-----------------------------------------------------------------------------

#endif
