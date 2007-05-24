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

#include "obj.hpp"
#include "data.hpp"
#include "glob.hpp"
#include "matrix.hpp"

//-----------------------------------------------------------------------------

void obj::obj::read_use(std::istream &lin, iset_m& is)
{
    std::string name;

    lin >> name;

    // Parse the material name and create a new mesh.

    meshes.push_back(ogl::mesh(name));

    // Reset the index cache.

    is.clear();
}

int obj::obj::read_fi(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv,
                                         ogl::vec3_v& nv, iset_m& is)
{
    iset_m::iterator ii;

    char cc;
    int  vi = 0;
    int  si = 0;
    int  ni = 0;
    int  val;

    // Read the next index set specification.

    std::string word;

    if ((lin >> word) == 0 || word.empty()) return -1;

    // Parse an index set.

    std::istringstream win(word);

    win >> vi >> cc >> si >> cc >> ni;

    // Convert face indices to vector cache indices.

    vi += (vi < 0) ? vv.size() : -1;
    si += (si < 0) ? sv.size() : -1;
    ni += (ni < 0) ? nv.size() : -1;

    // If we have not seen this index set before...

    iset key(vi, si, ni);

    if ((ii = is.find(key)) == is.end())
    {
        int val = int(meshes.back().vert_count());

        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, val));

        meshes.back().add_vert(ogl::vert(vv, sv, nv, vi, si, ni));
    }
    else val = ii->second;

    // Return the vertex index.

    return val;
}

void obj::obj::read_f(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv,
                                         ogl::vec3_v& nv, iset_m& is)
{
    std::vector<GLuint>           iv;
    std::vector<GLuint>::iterator ii;

    // Make sure we've got a mesh to add triangles to.
    
    if (meshes.empty()) meshes.push_back(ogl::mesh());

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_fi(lin, vv, sv, nv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Convert our N new vertex indices into N-2 new triangles.

    for (i = 0; i < n - 2; ++i)
        meshes.back().add_face(ogl::face(iv[0], iv[i + 1], iv[i + 2]));
}

//-----------------------------------------------------------------------------

int obj::obj::read_li(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv, iset_m& is)
{
    iset_m::iterator ii;

    char cc;
    int  vi = 0;
    int  si = 0;
    int  val;

    // Read the next index set specification.

    std::string word;

    if ((lin >> word) == 0 || word.empty()) return -1;

    // Parse an index set.

    std::istringstream win(word);

    win >> vi >> cc >> si;

    // Convert line indices to vector cache indices.

    vi += (vi < 0) ? vv.size() : -1;
    si += (si < 0) ? sv.size() : -1;

    // If we have not seen this index set before...

    iset key(vi, si, -1);

    if ((ii = is.find(key)) == is.end())
    {
        int val = int(meshes.back().vert_count());

        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, val));

        meshes.back().add_vert(ogl::vert(vv, sv, vv, vi, si, -1));
    }
    else val = ii->second;

    // Return the vertex index.

    return val;
}

void obj::obj::read_l(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv, iset_m& is)
{
    std::vector<GLuint>           iv;
    std::vector<GLuint>::iterator ii;

    // Make sure we've got a meshace to add lines to.
    
    if (meshes.empty()) meshes.push_back(ogl::mesh());

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_li(lin, vv, sv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Convert our N new vertex indices into N-1 new line.

    for (i = 0; i < n - 1; ++i)
        meshes.back().add_line(ogl::line(iv[i], iv[i + 1]));
}

//-----------------------------------------------------------------------------

void obj::obj::read_v(std::istream& lin, ogl::vec3_v& vv)
{
    ogl::vec3 v;

    lin >> v.v[0] >> v.v[1] >> v.v[2];

    vv.push_back(v);
}

void obj::obj::read_vt(std::istream& lin, ogl::vec2_v& sv)
{
    ogl::vec2 s;

    lin >> s.v[0] >> s.v[1];

    sv.push_back(s);
}

void obj::obj::read_vn(std::istream& lin, ogl::vec3_v& nv)
{
    ogl::vec3 n;

    lin >> n.v[0] >> n.v[1] >> n.v[2];

    nv.push_back(n);
}

//-----------------------------------------------------------------------------

obj::obj::obj(std::string name)
{
    // Initialize the input file.

    if (const char *buff = (const char *) ::data->load(name))
    {
        // Initialize the vector caches.

        ogl::vec3_v vv;
        ogl::vec2_v sv;
        ogl::vec3_v nv;

        iset_m is;

        // Parse each line of the file.

        std::istringstream sin(buff);
 
        std::string line;
        std::string key;

        while (std::getline(sin, line))
        {
            std::istringstream in(line);

            if (in >> key)
            {
                if      (key == "f")      read_f  (in, vv, sv, nv, is);
                else if (key == "l")      read_l  (in, vv, sv,     is);
                else if (key == "v")      read_v  (in, vv);
                else if (key == "vt")     read_vt (in, sv);
                else if (key == "vn")     read_vn (in, nv);
                else if (key == "usemtl") read_use(in, is);
            }
        }
    }

    // Release the open data file.

    ::data->free(name);

    // Initialize post-load state.

    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        i->calc_tangent();
}

obj::obj::~obj()
{
}

//-----------------------------------------------------------------------------

void obj::obj::box_bound(GLfloat *b) const
{
    float c[6];

    b[0] = std::numeric_limits<GLfloat>::max();
    b[1] = std::numeric_limits<GLfloat>::max();
    b[2] = std::numeric_limits<GLfloat>::max();
    b[3] = std::numeric_limits<GLfloat>::min();
    b[4] = std::numeric_limits<GLfloat>::min();
    b[5] = std::numeric_limits<GLfloat>::min();

    for (ogl::mesh_c i = meshes.begin(); i != meshes.end(); ++i)
    {
        i->box_bound(c);

        b[0] = std::min(b[0], c[0]);
        b[1] = std::min(b[1], c[1]);
        b[2] = std::min(b[2], c[2]);
        b[3] = std::max(b[3], c[3]);
        b[4] = std::max(b[4], c[4]);
        b[5] = std::max(b[5], c[5]);
    }
}

void obj::obj::sph_bound(GLfloat *b) const
{
    float c[1];

    b[0] = std::numeric_limits<GLfloat>::min();

    for (ogl::mesh_c i = meshes.begin(); i != meshes.end(); ++i)
    {
        i->sph_bound(c);

        b[0] = std::max(b[0], c[0]);
    }
}

//-----------------------------------------------------------------------------
