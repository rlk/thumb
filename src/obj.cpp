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
#include "data.hpp"
#include "glob.hpp"
#include "matrix.hpp"

//-----------------------------------------------------------------------------

void obj::obj::calc_tangent()
{
    // Assume all tangent vectors were zeroed during loading.  Enumerate faces.

    for (mesh_i mi = meshs.begin(); mi != meshs.end(); ++mi)
    {
        for (ogl::face_i fi = mi->faces.begin(); fi != mi->faces.end(); ++fi)
        {
            // Compute the vertex position differences.

            const float *vi = verts[fi->i].v.v;
            const float *vj = verts[fi->j].v.v;
            const float *vk = verts[fi->k].v.v;

            float dv0[3], dv1[3];

            dv0[0] = vj[0] - vi[0];
            dv0[1] = vj[1] - vi[1];
            dv0[2] = vj[2] - vi[2];

            dv1[0] = vk[0] - vi[0];
            dv1[1] = vk[1] - vi[1];
            dv1[2] = vk[2] - vi[2];

            // Compute the vertex texture coordinate differences.

            const float *si = verts[fi->i].s.v;
            const float *sj = verts[fi->j].s.v;
            const float *sk = verts[fi->k].s.v;

            float ds0[2], ds1[2];

            ds0[0] = sj[0] - si[0];
            ds0[1] = sj[1] - si[1];

            ds1[0] = sk[0] - si[0];
            ds1[1] = sk[1] - si[1];

            // Compute the tangent vector.

            float t[3];

            t[0] = ds1[1] * dv0[0] - ds0[1] * dv1[0];
            t[1] = ds1[1] * dv0[1] - ds0[1] * dv1[1];
            t[2] = ds1[1] * dv0[2] - ds0[1] * dv1[2];

            normalize(t);

            // Accumulate the vertex tangent vectors.

            float *ti = verts[fi->i].t.v;
            float *tj = verts[fi->j].t.v;
            float *tk = verts[fi->k].t.v;

            ti[0] += t[0];
            ti[1] += t[1];
            ti[2] += t[2];

            tj[0] += t[0];
            tj[1] += t[1];
            tj[2] += t[2];

            tk[0] += t[0];
            tk[1] += t[1];
            tk[2] += t[2];
        }
    }

    // Normalize all tangent vectors.

    for (ogl::vert_i vi = verts.begin(); vi != verts.end(); ++vi)
        normalize(vi->t.v);
}

//-----------------------------------------------------------------------------

void obj::obj::read_use(std::istream &lin)
{
    std::string name;

    lin >> name;

    meshs.push_back(mesh(name));
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
        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, (val = int(verts.size()))));

        verts.push_back(ogl::vert(vv, sv, nv, vi, si, ni));
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

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_fi(lin, vv, sv, nv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Make sure we've got a mesh to add triangles to.
    
    if (meshs.empty()) meshs.push_back(mesh());

    // Convert our N new vertex indices into N-2 new triangles.

    for (i = 0; i < n - 2; ++i)
        meshs.back().faces.push_back(ogl::face(iv[0], iv[i + 1], iv[i + 2]));
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
        // ... Create a new index set and vertex.

        is.insert(iset_m::value_type(key, (val = int(verts.size()))));

        verts.push_back(ogl::vert(vv, sv, vv, vi, si, -1));
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

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_li(lin, vv, sv, is)) >= 0)
        iv.push_back(GLuint(i));

    int n = iv.size();

    // Make sure we've got a meshace to add lines to.
    
    if (meshs.empty()) meshs.push_back(mesh());

    // Convert our N new vertex indices into N-1 new line.

    for (i = 0; i < n - 1; ++i)
        meshs.back().lines.push_back(ogl::line(iv[i], iv[i + 1]));
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
                else if (key == "usemtl") read_use(in);
            }
        }
    }

    // Release the open data file.

    ::data->free(name);

    // Initialize post-load state.

    calc_tangent();
}

obj::obj::~obj()
{
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

    for (ogl::vert_c vi = verts.begin(); vi != verts.end(); ++vi)
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
    b[0] = std::numeric_limits<GLfloat>::min();

    for (ogl::vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        GLfloat r = sqrt(vi->v.v[0] * vi->v.v[0] +
                         vi->v.v[1] * vi->v.v[1] +
                         vi->v.v[2] * vi->v.v[2]);

        b[0] = std::max(b[0], r);
    }
}

//-----------------------------------------------------------------------------
/*
GLsizei obj::obj::vsize() const
{
    return (verts.size() * sizeof (ogl::vert));
}

GLsizei obj::obj::esize() const
{
    GLsizei esz = 0;

    for (mesh_c si = meshs.begin(); si != meshs.end(); ++si)
        esz += (si->faces.size() * 3 * sizeof (GLuint) +
                si->lines.size() * 2 * sizeof (GLuint));

    return esz;
}

GLsizei obj::obj::vcopy(GLsizei off)
{
    GLsizei vsz = vsize();

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, off, vsz, &verts.front());

    return off + vsz;
}

GLsizei obj::obj::ecopy(GLsizei esz, GLsizei vsz)
{
    GLuint *ptr;
    GLuint  i0 = GLuint(vsz / sizeof (ogl::vert));
    GLuint  ii = GLuint(esz / sizeof (GLuint));

    // Upload element indices and note element ranges.

    if ((ptr = (GLuint *) glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
                                         GL_WRITE_ONLY)))
    {
        for (mesh_i si = meshs.begin(); si != meshs.end(); ++si)
        {
            // Convert face indices to global element buffer indices.

//          si->fp = ptr + ii;
            si->fp = (GLuint *) (ii * sizeof (GLuint));
            si->f0 = std::numeric_limits<GLuint>::max();
            si->fn = std::numeric_limits<GLuint>::min();

            ogl::face_c fi;

            for (fi = si->faces.begin(); fi != si->faces.end(); ++fi)
            {
                GLuint i = i0 + fi->i;
                GLuint j = i0 + fi->j;
                GLuint k = i0 + fi->k;

                ptr[ii++] = i;
                ptr[ii++] = j;
                ptr[ii++] = k;

                si->f0 = std::min(std::min(si->f0, i), std::min(j, k));
                si->fn = std::max(std::max(si->fn, i), std::max(j, k));
            }

            // Convert line indices to global element buffer indices.

//          si->lp = ptr + ii;
            si->lp = (GLuint *) (ii * sizeof (GLuint));
            si->l0 = std::numeric_limits<GLuint>::max();
            si->ln = std::numeric_limits<GLuint>::min();

            ogl::line_c li;

            for (li = si->lines.begin(); li != si->lines.end(); ++li)
            {
                GLuint i = i0 + li->i;
                GLuint j = i0 + li->j;

                ptr[ii++] = i;
                ptr[ii++] = j;

                si->l0 = std::min(std::min(si->l0, i), j);
                si->ln = std::max(std::max(si->ln, i), j);
            }
        }

        glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
    }

    return ii * sizeof (GLuint);
}
*/
//-----------------------------------------------------------------------------
