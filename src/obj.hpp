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
#include "program.hpp"
#include "mesh.hpp"

//-----------------------------------------------------------------------------

namespace obj
{
    //-------------------------------------------------------------------------

    struct iset
    {
        int vi;
        int si;
        int ni;

        iset(int v, int s, int n) : vi(v), si(s), ni(n) { }
    };

    struct icmp
    {
        bool operator()(const iset& A, const iset& B) const {
            if (A.vi < B.vi) return true;
            if (A.si < B.si) return true;
            if (A.ni < B.ni) return true;

            return false;
        }
    };

    typedef std::map<iset, int, icmp> iset_m;

    //-------------------------------------------------------------------------

    struct prop
    {
        int flags;

        prop(int f) : flags(f) { }

        virtual void draw(int) const = 0;

        virtual ~prop() { }
    };

    typedef const prop                         *prop_p;
    typedef std::vector<prop_p>                 prop_v;
    typedef std::vector<prop_p>::iterator       prop_i;
    typedef std::vector<prop_p>::const_iterator prop_c;

    //-------------------------------------------------------------------------

    struct mtrl
    {
        std::string name;
        
        prop_v props;
        int    flags;
        GLuint lite;
        GLuint dark;

        mtrl();
       ~mtrl();

        void draw(int) const;
        void init();
        void fini();
    };

    typedef const mtrl                       *mtrl_p;
    typedef std::vector<mtrl>                 mtrl_v;
    typedef std::vector<mtrl>::iterator       mtrl_i;
    typedef std::vector<mtrl>::const_iterator mtrl_c;

    //-------------------------------------------------------------------------

    struct surf
    {
        GLuint fibo;
        GLuint libo;

        mtrl_p state;

        ogl::face_v faces;
        ogl::line_v lines;

        GLuint *fp, f0, fn;
        GLuint *lp, l0, ln;

        surf(mtrl_p state) : fibo(0), libo(0), state(state) { }
        
        void draw(int) const;
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
        surf_v surfs;

        ogl::vert_v verts;

        void calc_tangent();

        // MTL read handlers.

        void read_map(std::istream&, prop&);
        void read_rgb(std::istream&, prop&);
        void read_mtl(std::istream&, std::string&);
        void read_use(std::istream&);

        // OBJ read handlers.

        int  read_fi(std::istream&, ogl::vec3_v&,
                                    ogl::vec2_v&,
                                    ogl::vec3_v&, iset_m&);
        void read_f (std::istream&, ogl::vec3_v&,
                                    ogl::vec2_v&,
                                    ogl::vec3_v&, iset_m&);
        int  read_li(std::istream&, ogl::vec3_v&,
                                    ogl::vec2_v&, iset_m&);
        void read_l (std::istream&, ogl::vec3_v&,
                                    ogl::vec2_v&, iset_m&);
        void read_v (std::istream&, ogl::vec3_v&);
        void read_vt(std::istream&, ogl::vec2_v&);
        void read_vn(std::istream&, ogl::vec3_v&);

    public:

        obj(std::string);
       ~obj();

        void box_bound(GLfloat *) const;
        void sph_bound(GLfloat *) const;
        
        GLsizei ecopy(GLsizei, GLsizei);
        GLsizei vcopy(GLsizei);
        GLsizei vsize() const;
        GLsizei esize() const;

        void draw(int) const;
        int  type(   ) const;

        void init();
        void fini();
    };
}

//-----------------------------------------------------------------------------

#endif
