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

#include <map>

#include "ogl-mesh.hpp"

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

    class obj
    {
        ogl::mesh_v meshes;

        // Read handlers.
/*
        void read_use(std::istream&, iset_m&);
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
*/
        const char *read_fi (const char *, ogl::vec3_d&,
                                           ogl::vec2_d&,
                                           ogl::vec3_d&, iset_m&, int &);
        const char *read_li (const char *, ogl::vec3_d&,
                                           ogl::vec2_d&, iset_m&, int &);

        const char *read_c  (const char *);
        const char *read_use(const char *, iset_m&);
        const char *read_f  (const char *, ogl::vec3_d&,
                                           ogl::vec2_d&,
                                           ogl::vec3_d&, iset_m&);
        const char *read_l  (const char *, ogl::vec3_d&,
                                           ogl::vec2_d&, iset_m&);
        const char *read_v  (const char *, ogl::vec3_d&);
        const char *read_vt (const char *, ogl::vec2_d&);
        const char *read_vn (const char *, ogl::vec3_d&);

        void center();

    public:

        obj(std::string, bool);
       ~obj();

        // Mesh accessors

        GLsizei          max_mesh()          const { return meshes.size(); }
        const ogl::mesh *get_mesh(GLsizei i) const { return meshes[i];     }
    };
}

//-----------------------------------------------------------------------------

#endif
