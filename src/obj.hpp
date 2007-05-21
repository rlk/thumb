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

    struct surf
    {
        std::string material;

        ogl::face_v faces;
        ogl::line_v lines;

        surf(std::string& m) : material(m) { }
    };

    typedef std::vector<surf>                 surf_v;
    typedef std::vector<surf>::iterator       surf_i;
    typedef std::vector<surf>::const_iterator surf_c;

    //-------------------------------------------------------------------------

    class obj
    {
             surf_v surfs;
        ogl::vert_v verts;

        void calc_tangent();

        // Read handlers.

        void read_use(std::istream&);
        int  read_fi (std::istream&, ogl::vec3_v&,
                                     ogl::vec2_v&,
                                     ogl::vec3_v&, iset_m&);
        void read_f  (std::istream&, ogl::vec3_v&,
                                     ogl::vec2_v&,
                                     ogl::vec3_v&, iset_m&);
        int  read_li (std::istream&, ogl::vec3_v&,
                                     ogl::vec2_v&, iset_m&);
        void read_l  (std::istream&, ogl::vec3_v&,
                                     ogl::vec2_v&, iset_m&);
        void read_v  (std::istream&, ogl::vec3_v&);
        void read_vt (std::istream&, ogl::vec2_v&);
        void read_vn (std::istream&, ogl::vec3_v&);

    public:

        obj(std::string);
       ~obj();

        // Bound calculators

        void box_bound(GLfloat *) const;
        void sph_bound(GLfloat *) const;

        // Batch data accessors

        GLsizei      count()                                   const;
        GLsizei      esize(GLsizei)                            const;
        GLsizei      vsize(GLsizei)                            const;
        void         ecopy(GLsizei, GLvoid *, GLuint)          const;
        void         vcopy(GLsizei, GLvoid *, const GLfloat *) const;
        std::string &state(GLsizei)                            const;
    };
}

//-----------------------------------------------------------------------------

#endif
