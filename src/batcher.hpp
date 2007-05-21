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

    class batch
    {
        // Vertex array parameters

        GLvoid *vptr;
        GLuint  vlen;

        // Element array parameters

        GLvoid *eptr;
        GLuint  elen;
        GLuint  emin;
        GLuint  emax;

    public:

        batch(std::string);
       ~batch();

        void draw() const;
    };

    typedef std::multi_map<binding *, batch *> batch_map;

    //-------------------------------------------------------------------------

    class element
    {
        GLfloat M[16];

        bool& dirty;

        const surface *srf;
        batch_vec bat;

    public:

        element(bool&, std::string);
       ~element();

        void move(const GLfloat *);

        // Batch data handlers

        GLsizei vsize() const;
        GLsizei esize() const;
        void    clean(GLvoid *, GLvoid *,
                      GLvoid *, GLvoid *, batch_map&, batch_map&);
    };

    typedef std::set<element *> element_set;

    //-------------------------------------------------------------------------

    class segment
    {
        GLfloat M[16];

        batch_map opaque;
        batch_map transp;

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

        GLsizei vsize() const;
        GLsizei esize() const;
        void    clean(GLvoid *, GLvoid *,
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
