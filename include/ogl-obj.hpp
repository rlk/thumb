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

#ifndef OBJ_HPP
#define OBJ_HPP

#include <map>
#include <vector>

#include <ogl-mesh.hpp>

//-----------------------------------------------------------------------------

namespace obj
{
    //-------------------------------------------------------------------------

    struct iset
    {
        int vi;
        int si;
        int ni;
        int ii;

        iset(int v, int s, int n, int i) : vi(v), si(s), ni(n), ii(i) { }
    };

    typedef std::vector<int>  indx_v;
    typedef std::vector<iset> iset_v;

    //-------------------------------------------------------------------------

    class obj
    {
        ogl::mesh_v meshes;

        // Reader caches.

        ogl::GLvec3_d vv;
        ogl::GLvec2_d sv;
        ogl::GLvec3_d nv;

        indx_v ii;
        iset_v is;

        double scale;

        // Read handlers.

        const char *read_fi (const char *, int&);
        const char *read_li (const char *, int&);

        const char *read_c  (const char *);
        const char *read_use(const char *);
        const char *read_f  (const char *);
        const char *read_l  (const char *);
        const char *read_v  (const char *);
        const char *read_vt (const char *);
        const char *read_vn (const char *);

        void center();

    public:

        obj(std::string, bool);
       ~obj();

        // Mesh accessors

        size_t           max_mesh()         const { return meshes.size(); }
        const ogl::mesh *get_mesh(size_t i) const { return meshes[i];     }
    };
}

//-----------------------------------------------------------------------------

#endif
