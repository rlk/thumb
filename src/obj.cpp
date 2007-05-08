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

namespace obj
{
    // Material color property ------------------------------------------------

    struct prop_col : public prop
    {
        GLenum  name;
        GLfloat c[4];

        prop_col(std::istream&, GLenum, int);

        void draw(int) const;
    };

    // Material shader property -----------------------------------------------

    struct prop_shd : public prop
    {
        const ogl::program *program;

        int flag;

        prop_shd(std::istream&, std::string&, int);
       ~prop_shd();

        void draw(int) const;
    };

    // Material map property --------------------------------------------------

    struct prop_map : public prop
    {
        const ogl::texture *texture;

        int unit;

        prop_map(std::istream&, std::string&, int, int);
       ~prop_map();

        void draw(int) const;
    };
}

//-----------------------------------------------------------------------------

obj::prop_col::prop_col(std::istream& lin, GLenum name, int f) :
    prop(f), name(name)
{
    lin >> c[0] >> c[1] >> c[2];

    c[3] = 1.0f;
}

void obj::prop_col::draw(int type) const
{
    if (type & flags)
        glMaterialfv(GL_FRONT_AND_BACK, name, c);
}

//-----------------------------------------------------------------------------

obj::prop_shd::prop_shd(std::istream& lin, std::string& path, int f) : prop(f)
{
    std::string vert_name;
    std::string frag_name;

    lin >> vert_name >> frag_name;

    program = ::glob->load_program(path + "/" + vert_name,
                                   path + "/" + frag_name);

    program->bind();
    {
        program->uniform("diffuse",     0);
        program->uniform("bump",        1);
        program->uniform("light",       2);
        program->uniform("shadow",      3);
        program->uniform("environment", 4);
        program->uniform("emission",    5);
        program->uniform("specular",    6);
        program->uniform("shininess",   7);
    }
    program->free();
}

obj::prop_shd::~prop_shd()
{
    if (program)
        ::glob->free_program(program);
}

void obj::prop_shd::draw(int type) const
{
    if (type & flags)
        if (program) program->bind();
}

//-----------------------------------------------------------------------------

obj::prop_map::prop_map(std::istream& lin, std::string& path, int unit, int f):
    prop(f), unit(unit)
{
    std::string name;

    lin >> name;

    texture = ::glob->load_texture(path + "/" + name);
}

obj::prop_map::~prop_map()
{
    if (texture)
        ::glob->free_texture(texture);
}

void obj::prop_map::draw(int type) const
{
    if (type & flags)
        if (texture) texture->bind(GL_TEXTURE0 + unit);
}

//-----------------------------------------------------------------------------

obj::mtrl::mtrl() : flags(0)
{
}

obj::mtrl::~mtrl()
{
    for (prop_i i = props.begin(); i != props.end(); ++i)
        delete (*i);
}

//-----------------------------------------------------------------------------

void obj::obj::calc_tangent()
{
    // Assume all tangent vectors were zeroed during loading.  Enumerate faces.

    for (surf_i si = surfs.begin(); si != surfs.end(); ++si)
    {
        for (face_i fi = si->faces.begin(); fi != si->faces.end(); ++fi)
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

    for (vert_i vi = verts.begin(); vi != verts.end(); ++vi)
        normalize(vi->t.v);
}

//-----------------------------------------------------------------------------

void obj::obj::read_mtl(std::istream& lin, std::string& path)
{
    // Parse the MTL file name from the string.

    std::string file;
    std::string name;

    lin >> file;

    name = path + "/" + file;

    // Initialize the input file.

    if (const char *buff = (const char *) ::data->load(name))
    {
        // Parse each line of the file.

        std::istringstream sin(buff);

        std::string line;
        std::string key;

        while (std::getline(sin, line))
        {
            std::istringstream in(line);

            if (in >> key)
            {
                const int L = DRAW_LIT;
                const int U = DRAW_UNLIT;

                prop_p P = 0;
                int    F = 0;

                if (key == "newmtl")
                {
                    mtrls.push_back(mtrl());
                    in >> mtrls.back().name;
                }

                // Texture map specifiers

                else if (key == "map_Kd") P = new prop_map(in, path, 0, U | L);
                else if (key == "bump")   P = new prop_map(in, path, 1, U | L);
                else if (key == "refl")   P = new prop_map(in, path, 4, U | L);
                else if (key == "map_Ke") P = new prop_map(in, path, 5, U);
                else if (key == "map_Ks") P = new prop_map(in, path, 6, L);
                else if (key == "map_Ns") P = new prop_map(in, path, 7, L);

                // Color specifiers

                else if (key == "Kd") P = new prop_col(in, GL_DIFFUSE,   L);
                else if (key == "Ka") P = new prop_col(in, GL_AMBIENT,   U);
                else if (key == "Ke") P = new prop_col(in, GL_EMISSION,  U);
                else if (key == "Ks") P = new prop_col(in, GL_SPECULAR,  L);
                else if (key == "Ns") P = new prop_col(in, GL_SHININESS, L);

                // Shader specifiers (non-standard)

                else if (key == "shader_unlit") P = new prop_shd(in, path, U);
                else if (key == "shader_lit")   P = new prop_shd(in, path, L);

                // Draw type specifiers (non-standard)

                else if (key == "lightsource") F = DRAW_LIGHTSOURCE;
                else if (key == "transparent") F = DRAW_TRANSPARENT;
                else if (key == "reflective")  F = DRAW_REFLECTIVE;
                else if (key == "refractive")  F = DRAW_REFRACTIVE;

                if (P) mtrls.back().props.push_back(P);
                if (F) mtrls.back().flags |= F;
            }
        }
    }

    // Release the open data file.

    ::data->free(name);
}

void obj::obj::read_use(std::istream &lin)
{
    std::string name;

    lin >> name;

    for (mtrl_c i = mtrls.begin(); i != mtrls.end(); ++i)
        if (i->name == name)
        {
            surfs.push_back(surf(&(*i)));
            break;
        }
}

//-----------------------------------------------------------------------------

int obj::obj::read_fi(std::istream& lin, vec3_v& vv,
                                         vec2_v& sv,
                                         vec3_v& nv,
                                         iset_m& is)
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

        verts.push_back(vert(vv, sv, nv, vi, si, ni));
    }
    else val = ii->second;

    // Return the vertex index.

    return val;
}

void obj::obj::read_f(std::istream& lin, vec3_v& vv,
                                         vec2_v& sv,
                                         vec3_v& nv,
                                         iset_m& is)
{
    std::vector<GLushort>           iv;
    std::vector<GLushort>::iterator ii;

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_fi(lin, vv, sv, nv, is)) >= 0)
        iv.push_back(GLushort(i));

    int n = iv.size();

    // Make sure we've got a surface to add triangles to.
    
    if (surfs.empty()) surfs.push_back(surf(0));

    // Convert our N new vertex indices into N-2 new triangles.

    for (i = 0; i < n - 2; ++i)
        surfs.back().faces.push_back(face(iv[0], iv[i + 1], iv[i + 2]));
}

//-----------------------------------------------------------------------------

int obj::obj::read_li(std::istream& lin, vec3_v& vv,
                                         vec2_v& sv,
                                         iset_m& is)
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

        verts.push_back(vert(vv, sv, vv, vi, si, -1));
    }
    else val = ii->second;

    // Return the vertex index.

    return val;
}

void obj::obj::read_l(std::istream& lin, vec3_v& vv,
                                         vec2_v& sv,
                                         iset_m& is)
{
    std::vector<GLushort>           iv;
    std::vector<GLushort>::iterator ii;

    // Scan the string, converting index sets to vertex indices.

    int i;

    while ((i = read_li(lin, vv, sv, is)) >= 0)
        iv.push_back(GLushort(i));

    int n = iv.size();

    // Make sure we've got a surface to add lines to.
    
    if (surfs.empty()) surfs.push_back(surf(0));

    // Convert our N new vertex indices into N-1 new line.

    for (i = 0; i < n - 1; ++i)
        surfs.back().lines.push_back(line(iv[i], iv[i + 1]));
}

//-----------------------------------------------------------------------------

void obj::obj::read_v(std::istream& lin, vec3_v& vv)
{
    vec3 v;

    lin >> v.v[0] >> v.v[1] >> v.v[2];

    vv.push_back(v);
}

void obj::obj::read_vt(std::istream& lin, vec2_v& sv)
{
    vec2 s;

    lin >> s.v[0] >> s.v[1];

    sv.push_back(s);
}

void obj::obj::read_vn(std::istream& lin, vec3_v& nv)
{
    vec3 n;

    lin >> n.v[0] >> n.v[1] >> n.v[2];

    nv.push_back(n);
}

//-----------------------------------------------------------------------------

obj::obj::obj(std::string name) : vbo(0)
{
    // Determine the path of the OBJ file.

    std::string::size_type psep = name.rfind("/");
    std::string            path = name;

    if (psep == std::string::npos)
        path.clear();
    else
        path.erase(psep);

    // Initialize the input file.

    if (const char *buff = (const char *) ::data->load(name))
    {
        // Initialize the vector caches.

        vec3_v vv;
        vec2_v sv;
        vec3_v nv;
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
                else if (key == "mtllib") read_mtl(in, path);
                else if (key == "usemtl") read_use(in);
            }
        }
    }

    // Release the open data file.

    ::data->free(name);

    // Initialize post-load state.

    calc_tangent();

    // Initialize the GL state.

    for (surf_i si = surfs.begin(); si != surfs.end(); ++si) si->init();
    for (mtrl_i mi = mtrls.begin(); mi != mtrls.end(); ++mi) mi->init();

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
    b[0] = std::numeric_limits<GLfloat>::min();

    for (vert_c vi = verts.begin(); vi != verts.end(); ++vi)
    {
        GLfloat r = sqrt(vi->v.v[0] * vi->v.v[0] +
                         vi->v.v[1] * vi->v.v[1] +
                         vi->v.v[2] * vi->v.v[2]);

        b[0] = std::max(b[0], r);
    }
}

//-----------------------------------------------------------------------------

int obj::obj::type() const
{
    int flags = 0;

    for (mtrl_c i = mtrls.begin(); i != mtrls.end(); ++i)
        flags |= i->flags;

    return flags ? flags : DRAW_OPAQUE;
}

//-----------------------------------------------------------------------------

#define OFFSET(i) ((char *) (i))

void obj::mtrl::draw(int type) const
{
    if ((type & flags) || ((type & DRAW_OPAQUE) && flags == 0))
    {
        if (type & DRAW_LIT)   glCallList(lite);
        if (type & DRAW_UNLIT) glCallList(dark);
    }
}

void obj::surf::draw(int type) const
{
    // Apply this surface's material.

    if (state) state->draw(type);

    // Draw this surface's faces.

    if (!faces.empty())
    {
        if (fibo)
        {
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, fibo);

            if (ogl::has_dre)
                glDrawRangeElementsEXT(GL_TRIANGLES, f0, fn, 3 * faces.size(),
                                       GL_UNSIGNED_SHORT, 0);
            else
                glDrawElements(GL_TRIANGLES, 3 * faces.size(),
                               GL_UNSIGNED_SHORT, 0);
        }
        else 
            glDrawElements(GL_TRIANGLES, 3 * faces.size(),
                           GL_UNSIGNED_SHORT, &faces.front());
    }

    // Draw this surface's lines.

    if (!lines.empty())
    {
        if (libo)
        {
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, libo);

            if (ogl::has_dre)
                glDrawRangeElementsEXT(GL_LINES, l0, ln, 2 * lines.size(),
                                       GL_UNSIGNED_SHORT, 0);
            else
                glDrawElements(GL_LINES, 2 * lines.size(),
                               GL_UNSIGNED_SHORT, 0);
        }
        else 
            glDrawElements(GL_LINES, 2 * lines.size(),
                           GL_UNSIGNED_SHORT, &lines.front());
    }
}

void obj::obj::draw(int type) const
{
    size_t s = sizeof (vert);

    // Bind the vertex buffers.

    if (vbo)
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

        glTexCoordPointer       (   2, GL_FLOAT,    s, OFFSET(36));
        glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, OFFSET(24));
        glNormalPointer         (      GL_FLOAT,    s, OFFSET(12));
        glVertexPointer         (   3, GL_FLOAT,    s, OFFSET( 0));
    }
    else
    {
        glTexCoordPointer       (   2, GL_FLOAT,    s, verts.front().s.v);
        glVertexAttribPointerARB(6, 3, GL_FLOAT, 0, s, verts.front().t.v);
        glNormalPointer         (      GL_FLOAT,    s, verts.front().n.v);
        glVertexPointer         (   3, GL_FLOAT,    s, verts.front().v.v);
    }

    // Render each surface

    for (surf_c i = surfs.begin(); i != surfs.end(); ++i)
        i->draw(type);
}

//-----------------------------------------------------------------------------

void obj::mtrl::init()
{
    // Initialize the display lists.

    if (!props.empty())
    {
        lite = glGenLists(1);
        dark = glGenLists(1);

        glNewList(lite, GL_COMPILE);
        {
            for (prop_c i = props.begin(); i != props.end(); ++i)
                (*i)->draw(DRAW_LIT);
        }
        glEndList();

        glNewList(dark, GL_COMPILE);
        {
            for (prop_c i = props.begin(); i != props.end(); ++i)
                (*i)->draw(DRAW_UNLIT);
        }
        glEndList();
    }
}

void obj::surf::init()
{
    if (ogl::has_vbo)
    {
        if (!faces.empty())
        {
            // Initialize the face element buffer range.

            f0 = std::numeric_limits<GLushort>::max();
            fn = std::numeric_limits<GLushort>::min();

            for (face_c i = faces.begin(); i != faces.end(); ++i)
            {
                f0 = std::min(f0, i->i);
                f0 = std::min(f0, i->j);
                f0 = std::min(f0, i->k);

                fn = std::max(fn, i->i);
                fn = std::max(fn, i->j);
                fn = std::max(fn, i->k);
            }

            // Initialize the face element buffer object.

            glGenBuffersARB(1, &fibo);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, fibo);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
                            faces.size() * sizeof (face),
                           &faces.front(), GL_STATIC_DRAW_ARB);
        }

        if (!lines.empty())
        {
            // Initialize the face element buffer range.

            l0 = std::numeric_limits<GLushort>::max();
            ln = std::numeric_limits<GLushort>::min();

            for (line_c i = lines.begin(); i != lines.end(); ++i)
            {
                l0 = std::min(l0, i->i);
                l0 = std::min(l0, i->j);

                ln = std::max(ln, i->i);
                ln = std::max(ln, i->j);
            }

            // Initialize the line element buffer object.

            glGenBuffersARB(1, &libo);
            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, libo);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,
                            lines.size() * sizeof (line),
                           &lines.front(), GL_STATIC_DRAW_ARB);
        }
    }
}

void obj::obj::init()
{
    // Initialize the vertex buffer object.

    if (ogl::has_vbo)
    {
        glGenBuffersARB(1, &vbo);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
        glBufferDataARB(GL_ARRAY_BUFFER_ARB,
                        verts.size() * sizeof (vert),
                       &verts.front(), GL_STATIC_DRAW_ARB);
    }
}

//-----------------------------------------------------------------------------

void obj::mtrl::fini()
{
    if (lite) glDeleteLists(lite, 1);
    if (dark) glDeleteLists(dark, 1);

    lite = 0;
    dark = 0;
}

void obj::surf::fini()
{
    // Delete the index buffer objects.

    if (ogl::has_vbo)
    {
        if (fibo) glDeleteBuffersARB(1, &fibo);
        if (libo) glDeleteBuffersARB(1, &libo);
    }

    fibo = 0;
    libo = 0;
}

void obj::obj::fini()
{
    // Delete the vertex buffer object.

    if (ogl::has_vbo)
    {
        if (vbo) glDeleteBuffersARB(1, &vbo);
    }

    vbo = 0;
}

//-----------------------------------------------------------------------------
