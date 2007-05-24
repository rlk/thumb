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

    class element;
    class segment;

    typedef std::set<element *>           element_s;
    typedef std::set<element *>::iterator element_i;
    typedef std::set<segment *>           segment_s;
    typedef std::set<segment *>::iterator segment_i;

    typedef std::map<const mesh *, vert *>                 mesh_vert_m;
    typedef std::map<const mesh *, vert *>::iterator       mesh_vert_i;
    typedef std::map<const mesh *, vert *>::const_iterator mesh_vert_c;

    typedef std::map<const mesh *, element *>                 mesh_elem_m;
    typedef std::map<const mesh *, element *>::iterator       mesh_elem_i;
    typedef std::map<const mesh *, element *>::const_iterator mesh_elem_c;

    //-------------------------------------------------------------------------

    struct batch
    {
        const binding *bnd;
        face          *ptr;

        GLsizei num;
        GLuint  min;
        GLuint  max;

        batch(const binding *b, face *p, GLsizei n) :
            bnd(b), ptr(p), num(n), min(0), max(0) { }
    };

    typedef std::vector<batch>                 batch_v;
    typedef std::vector<batch>::iterator       batch_i;
    typedef std::vector<batch>::const_iterator batch_c;

    //-------------------------------------------------------------------------
    // Batchable element

    class element
    {
        const surface *srf;
        mesh_vert_m vert_array;

        GLfloat M[16];

        bool& dirty;

    public:

        element(bool&, std::string);
       ~element();

        void move(const GLfloat *);

        // Batch data handlers

        GLsizei vcount() const;
        GLsizei ecount() const;

        void enlist(mesh_elem_m&,
                    mesh_elem_m&);

        vert *vert_cache(const mesh *, vert *);
    };

    //-------------------------------------------------------------------------
    // Batch segment

    class segment
    {
        element_s elements;

        batch_v opaque_batch;
        batch_v transp_batch;

        GLfloat M[16];

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
        GLsizei ecount() const;

        void reduce(vert_p, vert_p&, face_p, face_p&, mesh_elem_m&, batch_v&);
        void enlist(vert_p, vert_p&, face_p, face_p&);

        // Renderers

        void draw_opaque(bool) const;
        void draw_transp(bool) const;
    };

    //-------------------------------------------------------------------------
    // Batch manager

    class batcher
    {
        segment_s segments;

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
