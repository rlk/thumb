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
#include <sys/time.h>

#include "ogl-obj.hpp"
#include "ogl-aabb.hpp"
#include "app-data.hpp"
#include "app-glob.hpp"
#include "matrix.hpp"

//-----------------------------------------------------------------------------

static ogl::vec2 z2;
static ogl::vec3 z3;

//-----------------------------------------------------------------------------

void obj::obj::center()
{
    // Compute the axis-aligned bounding box of all meshes.

    ogl::aabb bound;

    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        (*i)->merge_bound(bound);

    // Compute the center of the bound.

    double c[3];
    bound.offset(c);

    // Offset the meshes to move the center to the origin.

    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        (*i)->apply_offset(c);
}

//-----------------------------------------------------------------------------
/*
void obj::obj::read_use(std::istream &lin, iset_m& is)
{
    std::string name;

    lin >> name;

    // Parse the material name and create a new mesh.

    meshes.push_back(new ogl::mesh(name));

    // Reset the index cache.

    is.clear();
}

int obj::obj::read_fi(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv,
                                         ogl::vec3_v& nv, iset_m& is)
{
    iset_m::iterator ii;

    char cc;
    int   i;
    int  vi = 0;
    int  si = 0;
    int  ni = 0;
    int val;

    // Read the next index set specification.

    std::string word;

    if ((lin >> word) == 0 || word.empty()) return -1;

    // TODO: profile and benchmark this
    // Parse an index set.  (This is probably unnecessary).

    std::istringstream win(word);

//  win >> vi >> cc >> si >> cc >> ni;

    // TODO: Optimize.  If v//n is detected, hop to a function doing only that.

    if (win >> i) vi = i; else win.clear();
    win >> cc;
    if (win >> i) si = i; else win.clear();
    win >> cc;
    if (win >> i) ni = i; else win.clear();

    // Convert face indices to vector cache indices.

    if (vi < 0) vi += vv.size(); else vi--;
    if (si < 0) si += sv.size(); else si--;
    if (ni < 0) ni += nv.size(); else ni--;

    // If we have not seen this index set before...

    iset key(vi, si, ni);

    if ((ii = is.find(key)) == is.end())
    {
        val = int(meshes.back()->count_verts());

        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, val));

        meshes.back()->add_vert((vi < 0) ? z3 : vv[vi],
                                (ni < 0) ? z3 : nv[ni],
                                (si < 0) ? z2 : sv[si]);
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
    
    if (meshes.empty()) meshes.push_back(new ogl::mesh());

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_fi(lin, vv, sv, nv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Convert our N new vertex indices into N-2 new triangles.

    for (i = 0; i < n - 2; ++i)
        meshes.back()->add_face(iv[0], iv[i + 1], iv[i + 2]);
}

//-----------------------------------------------------------------------------

int obj::obj::read_li(std::istream& lin, ogl::vec3_v& vv,
                                         ogl::vec2_v& sv, iset_m& is)
{
    iset_m::iterator ii;

    char cc;
    int  vi = 0;
    int  si = 0;
    int val;

    // Read the next index set specification.

    std::string word;

    if ((lin >> word) == 0 || word.empty()) return -1;

    // Parse an index set.

    std::istringstream win(word);

    win >> vi >> cc >> si;

    // Convert line indices to vector cache indices.

    if (vi < 0) vi += vv.size(); else vi--;
    if (si < 0) si += sv.size(); else si--;

    // If we have not seen this index set before...

    iset key(vi, si, -1);

    if ((ii = is.find(key)) == is.end())
    {
        val = int(meshes.back()->count_verts());

        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, val));

        meshes.back()->add_vert((vi < 0) ? z3 : vv[vi], z3,
                                (si < 0) ? z2 : sv[si]);
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
    
    if (meshes.empty()) meshes.push_back(new ogl::mesh());

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_li(lin, vv, sv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Convert our N new vertex indices into N-1 new line.

    for (i = 0; i < n - 1; ++i)
        meshes.back()->add_line(iv[i], iv[i + 1]);
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

obj::obj::obj(std::string name, bool center)
{
    struct timeval t0;
    struct timeval t1;

    // Initialize the input file.

    if (const char *buff = (const char *) ::data->load(name))
    {
        // Initialize the vector caches.

        ogl::vec3_v vv;
        ogl::vec2_v sv;
        ogl::vec3_v nv;

        iset_m is;

        // Parse each line of the file.

        gettimeofday(&t0, 0);

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

        gettimeofday(&t1, 0);
    }

    printf("%s %f\n", name.c_str(), (double(t1.tv_sec  - t0.tv_sec) + 
                                     double(t1.tv_usec - t0.tv_usec) * 0.000001));

    // Release the open data file.

    ::data->free(name);

    // Initialize post-load state.

    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        (*i)->calc_tangent();

    // Optionally center the object about the origin.

    if (center)
    {
        ogl::aabb bound;
        ogl::mesh_i i;
        double c[3];

        for (i = meshes.begin(); i != meshes.end(); ++i)
            (*i)->merge_bound(bound);

        bound.offset(c);

        for (i = meshes.begin(); i != meshes.end(); ++i)
            (*i)->apply_offset(c);
    }
}
*/
obj::obj::~obj()
{
    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

static bool token_c(const char *p)
{
    return (*p && p[0] == '#');
}

static bool token_f(const char *p)
{
    return (*p && p[0] == 'f' && isspace(p[1]));
}

static bool token_l(const char *p)
{
    return (*p && p[0] == 'l' && isspace(p[1]));
}

static bool token_v(const char *p)
{
    return (*p && p[0] == 'v' && isspace(p[1]));
}

static bool token_vn(const char *p)
{
    return (*p && p[0] == 'v' && p[1] == 'n' && isspace(p[2]));
}

static bool token_vt(const char *p)
{
    return (*p && p[0] == 'v' && p[1] == 't' && isspace(p[2]));
}

static bool token_use(const char *p)
{
    return (*p && !strncmp(p, "usemtl", 6));
}

static const char *scannl(const char *p)
{
    while (1)
        switch (*p)
        {
        case '\0': return p + 0;
        case '\n': return p + 1;
        default  : p++;
        }
}

//-----------------------------------------------------------------------------

const char *obj::obj::read_fi(const char *p, ogl::vec3_d& vv,
                                             ogl::vec2_d& sv,
                                             ogl::vec3_d& nv,
                                             iset_m& is, int& i)
{
    // Read the next index set specification.

    int  vi = 0;
    int  si = 0;
    int  ni = 0;
    char *q;

    {                vi = int(strtol(  p, &q, 0)); p = q; }
    if (*p == '/') { si = int(strtol(++p, &q, 0)); p = q; }
    if (*p == '/') { ni = int(strtol(++p, &q, 0)); p = q; }

    if (vi)
    {
        // Convert face indices to vector cache indices.

        if (vi < 0) vi += vv.size(); else vi--;
        if (si < 0) si += sv.size(); else si--;
        if (ni < 0) ni += nv.size(); else ni--;

        // If we have not seen this index set before...

        iset key(vi, si, ni);
        iset_m::iterator ii;

        if ((ii = is.find(key)) == is.end())
        {
            i = int(meshes.back()->count_verts());

            // ... Create a new index set and vertex.

            is.insert(iset_m::value_type(key, i));

            meshes.back()->add_vert((vi < 0) ? z3 : vv[vi],
                                    (ni < 0) ? z3 : nv[ni],
                                    (si < 0) ? z2 : sv[si]);
        }
        else i = ii->second;
    }
    else i = -1;

    return p;
}

const char *obj::obj::read_li(const char *p, ogl::vec3_d& vv,
                                             ogl::vec2_d& sv,
                                             iset_m& is, int& i)
{
    // Read the next index set specification.

    int  vi = 0;
    int  si = 0;
    char *q;

    {                vi = int(strtol(  p, &q, 0)); p = q; }
    if (*p == '/') { si = int(strtol(++p, &q, 0)); p = q; }

    if (vi)
    {
        // Convert face indices to vector cache indices.

        if (vi < 0) vi += vv.size(); else vi--;
        if (si < 0) si += sv.size(); else si--;

        // If we have not seen this index set before...

        iset key(vi, si, -1);
        iset_m::iterator ii;

        if ((ii = is.find(key)) == is.end())
        {
            i = int(meshes.back()->count_verts());

            // ... Create a new index set and vertex.

            is.insert(iset_m::value_type(key, i));

            meshes.back()->add_vert((vi < 0) ? z3 : vv[vi], z3,
                                    (si < 0) ? z2 : sv[si]);
        }
        else i = ii->second;
    }
    else i = -1;

    return p;
}

//-----------------------------------------------------------------------------

const char *obj::obj::read_use(const char *p, iset_m& is)
{
    std::string name = "default";

    // Parse the material name and create a new mesh.

    meshes.push_back(new ogl::mesh(name));

    // Reset the index cache.
    
    is.clear();

    return scannl(p);
}

//-----------------------------------------------------------------------------

const char *obj::obj::read_c(const char *p)
{
    while (token_c(p))
        p = scannl(p);

    return p;
}

const char *obj::obj::read_v(const char *p, ogl::vec3_d& vv)
{
    ogl::vec3 v;
    char     *q;

    // Process a sequence of vertex positions.

    while (token_v(p))
    {
        v.v[0] = strtof(p + 1, &q); p = q;
        v.v[1] = strtof(p,     &q); p = q;
        v.v[2] = strtof(p,     &q); p = scannl(q);

        vv.push_back(v);
    }
    return p;
}

const char *obj::obj::read_vt(const char *p, ogl::vec2_d& sv)
{
    ogl::vec2 v;
    char     *q;

    // Process a sequence of vertex texture coordinaces.

    while (token_vt(p))
    {
        v.v[0] = strtof(p + 2, &q); p = q;
        v.v[1] = strtof(p,     &q); p = scannl(q);

        sv.push_back(v);
    }
    return p;
}

const char *obj::obj::read_vn(const char *p, ogl::vec3_d& nv)
{
    ogl::vec3 v;
    char     *q;

    // Process a sequence of vertex normals.

    while (token_vn(p))
    {
        v.v[0] = strtof(p + 2, &q); p = q;
        v.v[1] = strtof(p,     &q); p = q;
        v.v[2] = strtof(p,     &q); p = scannl(q);

        nv.push_back(v);
    }
    return p;
}

//-----------------------------------------------------------------------------

const char *obj::obj::read_f(const char *p, ogl::vec3_d& vv,
                                            ogl::vec2_d& sv,
                                            ogl::vec3_d& nv, iset_m& is)
{
    // Make sure we've got a mesh to receive faces.
    
    if (meshes.empty())
        meshes.push_back(new ogl::mesh());

    // Process a sequence of face definitions.

    while (token_f(p))
    {
        std::vector<GLuint>           iv;
        std::vector<GLuint>::iterator ii;

        p++;

        // Scan the string, converting index sets to vertex indices.

        int i;

        while ((p = read_fi(p, vv, sv, nv, is, i)) && i >= 0)
            iv.push_back(GLuint(i));

        p = scannl(p);

        // Convert our N new vertex indices into N-2 new triangles.

        int n = iv.size();

        for (i = 0; i < n - 2; ++i)
            meshes.back()->add_face(iv[0], iv[i + 1], iv[i + 2]);
    }
    return p;
}

const char *obj::obj::read_l(const char *p, ogl::vec3_d& vv,
                                            ogl::vec2_d& sv, iset_m& is)
{
    // Make sure we've got a mesh to receive lines.
    
    if (meshes.empty())
        meshes.push_back(new ogl::mesh());

    // Process a sequence of line definitions.

    while (token_l(p))
    {
        std::vector<GLuint>           iv;
        std::vector<GLuint>::iterator ii;

        p++;

        // Scan the string, converting index sets to vertex indices.

        int i;

        while ((p = read_li(p, vv, sv, is, i)) && i >= 0)
            iv.push_back(GLuint(i));

        p = scannl(p);

        // Convert our N new vertex indices into N-1 new lines.

        int n = iv.size();

        for (i = 0; i < n - 1; ++i)
            meshes.back()->add_line(iv[i], iv[i + 1]);
    }
    return p;
}

//-----------------------------------------------------------------------------

obj::obj::obj(std::string name, bool c)
{
    struct timeval t0;
    struct timeval t1;

    // Initialize the input file.

    if (const char *p = (const char *) ::data->load(name))
    {
        // Initialize the vector caches.

        ogl::vec3_d vv;
        ogl::vec2_d sv;
        ogl::vec3_d nv;

        iset_m is;

        // Process data until the end of the file is reached.

        gettimeofday(&t0, 0);

        while (*p)
        {
            if      (token_c  (p)) p = read_c  (p);
            else if (token_f  (p)) p = read_f  (p, vv, sv, nv, is);
            else if (token_l  (p)) p = read_l  (p, vv, sv,     is);
            else if (token_v  (p)) p = read_v  (p, vv);
            else if (token_vt (p)) p = read_vt (p, sv);
            else if (token_vn (p)) p = read_vn (p, nv);
            else if (token_use(p)) p = read_use(p, is);
            else                   p = scannl(p);
        }

        gettimeofday(&t1, 0);
    }

    printf("%s %f\n", name.c_str(), (double(t1.tv_sec  - t0.tv_sec) + 
                                     double(t1.tv_usec - t0.tv_usec) * 0.000001));

    // Release the open data file.

    ::data->free(name);

    // Initialize post-load state.

    for (ogl::mesh_i i = meshes.begin(); i != meshes.end(); ++i)
        (*i)->calc_tangent();

    // Optionally center the object about the origin.

    if (c) center();
}

//-----------------------------------------------------------------------------

