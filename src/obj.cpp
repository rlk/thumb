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

#include <iostream>
#include <sstream>
#include <cmath>

#include "obj.hpp"
#include "main.hpp"

//-----------------------------------------------------------------------------

int obj::obj::read_i(std::istream& lin, vec3_v& vv,
                                        vec2_v& tv,
                                        vec3_v& nv,
                                        iset_m& is, int gi)
{
    iset_m::iterator ii;

    char cc;
    int  vi = 0;
    int  ti = 0;
    int  ni = 0;
    int   i;

    // Attempt to parse an index set.

    if ((lin >> vi) == 0) return -1;
    lin >> cc;
    if ((lin >> ti) == 0) lin.clear();
    lin >> cc;
    if ((lin >> ni) == 0) lin.clear();

    // Convert face indices to vector cache indices.

    vi += (vi < 0) ? vv.size() : -1;
    ti += (ti < 0) ? tv.size() : -1;
    ni += (ni < 0) ? nv.size() : -1;

    // If we have not seen this index set before...

    if ((ii = is.find(iset(vi, ti, ni, gi))) == is.end())
    {
        // ... Create a new index set and vertex.

        is[iset(vi, ti, ni, gi)] = (i = int(verts.size()));

        verts.push_back(vert(vv, tv, nv, vi, ti, ni));
    }
    else i = ii->second;

    // Return the vertex index.

    return i;
}

void obj::obj::read_f(std::istream& lin, vec3_v& vv,
                                         vec2_v& tv,
                                         vec3_v& nv,
                                         iset_m& is, int gi)
{
    std::vector<GLushort>           iv;
    std::vector<GLushort>::iterator ii;

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_i(lin, vv, tv, nv, is, gi)) >= 0)
        iv.push_back(GLushort(i));

    int n = iv.size();

    // Convert our N new vertex indices into N-2 new triangles.

    for (i = 0; i < n - 2; ++i)
        surfs.back().faces.push_back(face(iv[0], iv[i + 1], iv[i + 2]));
}

//-----------------------------------------------------------------------------

void obj::obj::read_v(std::istream& lin, vec3_v& vv)
{
    vec3 v;

    lin >> v.v[0] >> v.v[1] >> v.v[2];

    vv.push_back(v);
}

void obj::obj::read_vt(std::istream& lin, vec2_v& tv)
{
    vec2 t;

    lin >> t.v[0] >> t.v[1];

    tv.push_back(t);
}

void obj::obj::read_vn(std::istream& lin, vec3_v& nv)
{
    vec3 n;

    lin >> n.v[0] >> n.v[1] >> n.v[2];

    nv.push_back(n);
}

//-----------------------------------------------------------------------------

obj::obj::obj(std::string name)
{
    // Initialize the input file.

    std::istringstream sin((const char *) ::data->load_dat(name));
 
   // Initialize the vector caches.

    vec3_v vv;
    vec2_v tv;
    vec3_v nv;
    iset_m is;
    int    gi = 0;

    // Initialize the default material and surface.

    mtrl M;
    surf S;

    mtrls.push_back(M);
    surfs.push_back(S);

    // Parse each line of the file.

    std::string line;
    std::string type;

    while (std::getline(sin, line))
    {
        std::istringstream lin(line);

        lin >> type;

        if      (type == "f")  read_f (lin, vv, tv, nv, is, gi);
        else if (type == "v")  read_v (lin, vv);
        else if (type == "vt") read_vt(lin, tv);
        else if (type == "vn") read_vn(lin, nv);
        else if (type == "s")  lin >> gi;
    }

    // Release the open data file.

    ::data->free_dat(name);

    // Initialize the GL state.

    for (surf_i si = surfs.begin(); si != surfs.end(); ++si)
        si->init();

    init();
}

obj::obj::~obj()
{
    // Finalize the GL state.

    for (surf_i si = surfs.begin(); si != surfs.end(); ++si)
        si->fini();

    fini();
}

//-----------------------------------------------------------------------------

void obj::obj::box_bound(GLfloat *b) const
{
    b[0] = std::numeric_limits<GLfloat>::max();
    b[1] = std::numeric_limits<GLfloat>::max();
    b[2] = std::numeric_limits<GLfloat>::max();
    b[3] = std::numeric_limits<GLfloat>::min();
    b[4] = std::numeric_limits<GLfloat>::min();
    b[5] = std::numeric_limits<GLfloat>::min();

    for (vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        b[0] = std::min(b[0], vi->v.v[0]);
        b[1] = std::min(b[1], vi->v.v[1]);
        b[2] = std::min(b[2], vi->v.v[2]);
        b[3] = std::max(b[3], vi->v.v[0]);
        b[4] = std::max(b[4], vi->v.v[1]);
        b[5] = std::max(b[5], vi->v.v[2]);
    }
}

void obj::obj::sph_bound(GLfloat *b) const
{
    for (vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        GLfloat r = sqrt(vi->v.v[0] * vi->v.v[0] +
                         vi->v.v[1] * vi->v.v[1] +
                         vi->v.v[2] * vi->v.v[2]);

        b[0] = std::max(b[0], r);
    }
}

//-----------------------------------------------------------------------------

void obj::surf::init()
{
    // Initialize the index buffer object.

    glGenBuffersARB(1, &ibo);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
                    faces.size() * sizeof (face),
                   &faces.front(), GL_STATIC_DRAW_ARB);
}

void obj::obj::init()
{
    // Initialize the vertex buffer object.

    glGenBuffersARB(1, &vbo);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                    verts.size() * sizeof (vert),
                   &verts.front(), GL_STATIC_DRAW_ARB);
}

//-----------------------------------------------------------------------------

void obj::surf::fini()
{
    // Delete the index buffer object.

    glDeleteBuffersARB(1, &ibo);
    ibo = 0;
}

void obj::obj::fini()
{
    // Delete the vertex buffer object.

    glDeleteBuffersARB(1, &vbo);
    vbo = 0;
}

//-----------------------------------------------------------------------------

#define OFFSET(i) ((char *) (i))

void obj::prop::draw() const
{
}

void obj::mtrl::draw() const
{
    for (prop_c pi = props.begin(); pi != props.end(); ++pi)
        pi->draw();
}

void obj::surf::draw() const
{
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);
    glDrawElements(GL_TRIANGLES, 3 * faces.size(), GL_UNSIGNED_SHORT, 0);
}

void obj::obj::draw() const
{
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT);
    {
        size_t s = sizeof (vert);

        // Bind the vertex buffer.

        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

        glNormalPointer  (   GL_FLOAT, s, OFFSET( 0));
        glTexCoordPointer(2, GL_FLOAT, s, OFFSET(12));
        glVertexPointer  (3, GL_FLOAT, s, OFFSET(20));

        // Render each surface

        for (surf_c si = surfs.begin(); si != surfs.end(); ++si)
            si->draw();
    }
    glPopAttrib();
    glPopClientAttrib();
}

//-----------------------------------------------------------------------------
