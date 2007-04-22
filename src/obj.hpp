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

#ifndef OBJ_HPP
#define OBJ_HPP

#include <string>
#include <vector>
#include <list>
#include <map>

#include "opengl.hpp"
#include "texture.hpp"

//-----------------------------------------------------------------------------

namespace obj
{
    //-------------------------------------------------------------------------

    struct vec2
    {
        GLfloat v[2];

        vec2(GLfloat s, GLfloat t) {
            v[0] = s;
            v[1] = t;
        }
        vec2() { }
    };

    struct vec3
    {
        GLfloat v[3];

        vec3(GLfloat x, GLfloat y, GLfloat z) {
            v[0] = x;
            v[1] = y;
            v[2] = z;
        }
        vec3() { }
    };

    typedef std::vector<vec2> vec2_v;
    typedef std::vector<vec3> vec3_v;

    //-------------------------------------------------------------------------

    struct iset
    {
        int vi;
        int ti;
        int ni;
        int gi;

        iset(int v, int t, int n, int g) : vi(v), ti(t), ni(n), gi(g) { }
    };

    struct icmp
    {
        bool operator()(const iset& A, const iset& B) const {
            if (A.vi < B.vi) return true;
            if (A.ti < B.ti) return true;
            if (A.ni < B.ni) return true;
            if (A.gi < B.gi) return true;

            return false;
        }
    };

    typedef std::map<iset, int, icmp> iset_m;

    //-------------------------------------------------------------------------

    struct vert
    {
        vec3 n;
        vec2 t;
        vec3 v;

        vert() { }

        vert(vec3_v& vv, vec2_v& tv, vec3_v& nv, int vi, int ti, int ni) {
            v = (vi >= 0) ? vv[vi] : vec3(0.0f, 0.0f, 0.0f);
            t = (ti >= 0) ? tv[ti] : vec2(0.0f, 0.0f);
            n = (ni >= 0) ? nv[ni] : vec3(0.0f, 0.0f, 0.0f);
        }
    };

    typedef std::vector<vert>                 vert_v;
    typedef std::vector<vert>::iterator       vert_i;
    typedef std::vector<vert>::const_iterator vert_c;

    //-------------------------------------------------------------------------

    struct face
    {
        GLushort i;
        GLushort j;
        GLushort k;

        face()                                                      { }
        face(GLushort I, GLushort J, GLushort K) : i(I), j(J), k(K) { }
    };

    typedef std::vector<face>                 face_v;
    typedef std::vector<face>::iterator       face_i;
    typedef std::vector<face>::const_iterator face_c;

    //-------------------------------------------------------------------------

    struct prop
    {
        virtual ~prop() { }
        virtual void draw() const = 0;
    };

    struct prop_col : public prop
    {
        GLenum  name;
        GLfloat c[4];

        prop_col(std::istream&, GLenum);

        void draw() const;
    };

    struct prop_map : public prop
    {
        const ogl::texture *texture;

        GLenum unit;

        prop_map(std::istream&, std::string&, GLenum);
       ~prop_map();

        void draw() const;
    };

    typedef const prop                         *prop_p;
    typedef std::list<prop_p>                 prop_v;
    typedef std::list<prop_p>::iterator       prop_i;
    typedef std::list<prop_p>::const_iterator prop_c;

    //-------------------------------------------------------------------------

    struct mtrl
    {
        std::string name;

        GLfloat alpha;
        prop_v  props;

        mtrl();
       ~mtrl();

        void draw() const;
    };

    typedef const mtrl                       *mtrl_p;
    typedef std::vector<mtrl>                 mtrl_v;
    typedef std::vector<mtrl>::iterator       mtrl_i;
    typedef std::vector<mtrl>::const_iterator mtrl_c;

    //-------------------------------------------------------------------------

    struct surf
    {
        GLuint ibo;

        mtrl_p state;
        face_v faces;

        surf(mtrl_p state) : ibo(0), state(state) { }
        
        void draw() const;
        void init();
        void fini();
    };

    typedef std::vector<surf>                 surf_v;
    typedef std::vector<surf>::iterator       surf_i;
    typedef std::vector<surf>::const_iterator surf_c;

    //-------------------------------------------------------------------------

    class obj
    {
        GLuint vbo;

        mtrl_v mtrls;
        vert_v verts;
        surf_v surfs;

        void read_map(std::istream&, prop&);
        void read_rgb(std::istream&, prop&);
        void read_mtl(std::istream&, std::string&);
        void read_use(std::istream&);

        int  read_i (std::istream&, vec3_v&, vec2_v&, vec3_v&, iset_m&, int);
        void read_f (std::istream&, vec3_v&, vec2_v&, vec3_v&, iset_m&, int);
        void read_v (std::istream&, vec3_v&);
        void read_vt(std::istream&, vec2_v&);
        void read_vn(std::istream&, vec3_v&);

    public:

        obj(std::string);
       ~obj();

        void box_bound(GLfloat *) const;
        void sph_bound(GLfloat *) const;
        
        void draw() const;
        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
