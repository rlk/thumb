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

#ifndef BATCHER_HPP
#define BATCHER_HPP

#include <map>
#include <set>

#include "binding.hpp"
#include "surface.hpp"

//-----------------------------------------------------------------------------

#define MAXPRG 2
#define MAXTEX 8

namespace ogl
{
    //-------------------------------------------------------------------------

    struct batch
    {
        const mesh *M;

        // Vertex array parameters

        GLvoid *vptr;
        GLuint  vnum;

        // Face element array parameters

        GLvoid *fptr;
        GLuint  fnum;
        GLuint  fmin;
        GLuint  fmax;

        // Line element array parameters

        GLvoid *lptr;
        GLuint  lnum;
        GLuint  lmin;
        GLuint  lmax;
    };

    typedef std::vector<batch> batch_v;

    //-------------------------------------------------------------------------

    typedef std::set<mesh *, mesh_cmp> mesh_set;

    class element
    {
        GLfloat M[16];

        bool& dirty;

        ogl::mesh_v meshs;

    public:

        element(bool&, std::string);
       ~element();

        void move(const GLfloat *);

        // Batch data handlers

        GLsizei vcount() const;
        GLsizei icount() const;
        void    enlist(mesh_set& opaque,
                       mesh_set& transp);
    };

    typedef std::set<element *> element_set;

    //-------------------------------------------------------------------------

    class segment
    {
        GLfloat M[16];

        element_set elements;

        bool& dirty;

    public:

        segment(bool&);
       ~segment();

        // Element handlers

        element *add(std::string);
        void     del(element *);

        void move(const GLfloat *);

        // Batch data handlers

        GLsizei vcount() const;
        GLsizei icount() const;
        void    enlist(GLvoid *, GLvoid *,
                       GLvoid *, GLvoid *);

        // Renderers

        void draw_opaque(bool) const;
        void draw_transp(bool) const;
    };

    typedef std::set<segment *> segment_set;

    //-------------------------------------------------------------------------

    class batcher
    {
        segment_set segments;

        GLuint vbo;
        GLuint ebo;

        bool dirty;
        void clean();

    public:
        
        batcher();
       ~batcher();

        // Segment handlers

        segment *add(         );
        void     del(segment *);

        // Renderers

        void draw_init();
        void draw_fini();

        void draw_opaque(bool) const;
        void draw_transp(bool) const;
    };
}

//-----------------------------------------------------------------------------

#endif
