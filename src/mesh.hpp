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

//-----------------------------------------------------------------------------

// Draw property flags.

#define DRAW_LIGHTSOURCE  1
#define DRAW_TRANSPARENT  2
#define DRAW_OPAQUE       4
#define DRAW_REFLECTIVE   8
#define DRAW_REFRACTIVE  16
#define DRAW_LIT         32
#define DRAW_UNLIT       64
#define DRAW_GIZMO      128

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

    struct vert
    {
        vec3    v;
        vec3    n;
        vec3    t;
        vec2    s;
        GLubyte w[4];

        vert() { }

        vert(vec3_v& vv, vec2_v& sv, vec3_v& nv, int vi, int si, int ni) {
            v = (vi >= 0) ? vv[vi] : vec3();
            s = (si >= 0) ? sv[si] : vec2();
            n = (ni >= 0) ? nv[ni] : vec3();
            t =                      vec3();
            w[0] = w[1] = w[2] = w[3] = 0;
        }
    };

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

    typedef std::vector<line>                 line_v;
    typedef std::vector<line>::iterator       line_i;
    typedef std::vector<line>::const_iterator line_c;
}

//-----------------------------------------------------------------------------

#endif
