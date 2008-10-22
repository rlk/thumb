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

#ifndef GEOGEN_HPP
#define GEOGEN_HPP

#include "ogl-opengl.hpp"
#include "ogl-program.hpp"
#include "default.hpp"
#include "ogl-buffer.hpp"
#include "ogl-image.hpp"
#include "ogl-frame.hpp"
#include "ogl-lut.hpp"

namespace uni
{
    //-------------------------------------------------------------------------
    // Icosahedron topology definition

    class geoico
    {
        static int    _point_i[20][3];
        static int    _patch_i[20][3];
        static int    _patch_j[20][3];
        static double _point_v[12][3];
        static double _patch_c[20][3][2];

    public:

        geoico();

        const int    *point_i(int p)        const { return _point_i[p];    }
        const int    *patch_i(int p)        const { return _patch_i[p];    }
        const int    *patch_j(int p)        const { return _patch_j[p];    }
        const double *point_v(int p)        const { return _point_v[p];    }
        const double *patch_c(int p, int i) const { return _patch_c[p][i]; }
    };

    //-------------------------------------------------------------------------
    // Geometry cache patch connectivity

    class geocon
    {
        GLushort *index;
        GLsizei   n;

    public:

        geocon(GLsizei);
        ~geocon();

        GLushort get(GLsizei, GLsizei);
    };

    //-------------------------------------------------------------------------
    // Geometry cache bisection lookup table

    class geolut : public ogl::image
    {
        void subdiv(GLushort *, GLsizei, GLsizei, GLsizei,
                                         GLsizei, GLsizei,
                                         GLsizei, GLsizei, geocon&);
    public:

        geolut(GLsizei, geocon&);
    };

    //-------------------------------------------------------------------------
    // Geometry cache index buffer

    class geoidx : public ogl::buffer
    {
        GLushort *append(GLushort *, GLsizei, GLsizei,
                                     GLsizei, GLsizei,
                                     GLsizei, GLsizei, geocon&);
        GLushort *subdiv(GLushort *, GLsizei, GLsizei, GLsizei, int,
                                              GLsizei, GLsizei, int,
                                              GLsizei, GLsizei, int, geocon&);
    public:

        geoidx(GLsizei, geocon&);
    };

    //-------------------------------------------------------------------------
    // Geometry cache radius correction function lookup table.

    class georad : public ogl::lut
    {
        GLsizei n;

        double func(double);

    public:

        georad(GLsizei=1024);
    };

    //-------------------------------------------------------------------------
    // Geometry cache shared data.

    class geodat
    {
        GLsizei d;

        geoico _ico;
        georad _rad;
        geocon _con;
        geolut _lut;
        geoidx _idx;

    public:

        geodat(GLsizei=DEFAULT_PATCH_DEPTH);

        GLsizei depth() const { return d; }

        GLsizei vtx_len() const;
        GLsizei idx_len() const;

        const geoico *ico() const { return &_ico; }
        const georad *rad() const { return &_rad; }
        const geocon *con() const { return &_con; }
        const geolut *lut() const { return &_lut; }
        const geoidx *idx() const { return &_idx; }
    };

    //-------------------------------------------------------------------------
    // Geometry cache vector buffer

    class geobuf
    {
        ogl::frame *ping;
        ogl::frame *pong;

    protected:

        ogl::frame *src;
        ogl::frame *dst;

        GLsizei w;
        GLsizei h;

        const ogl::program *copy;
        const ogl::program *calc;
        const ogl::program *show;

    public:

        geobuf(GLsizei, GLsizei, std::string,
                                 std::string,
                                 std::string,
                                 std::string);
       ~geobuf();

        void bind_proc() const;
        void free_proc() const;

        void bind_frame()      const { src->bind(); }
        void free_frame()      const { src->free(); }

        void bind(GLenum unit) const { src->bind_color(unit); }
        void free(GLenum unit) const { src->free_color(unit); }

        void draw(int, int) const;
        void null()         const;
        void swap();
    };

    //-------------------------------------------------------------------------
    // Geometry cache vector generator

    class geogen : public geobuf
    {
        GLsizei  d;
        GLfloat *p;

        ogl::buffer buff;

    public:

        geogen(GLsizei, GLsizei, std::string,
                                 std::string,
                                 std::string,
                                 std::string);
        virtual ~geogen() { }

        virtual void init();
        virtual void seed(GLsizei, GLfloat=0, GLfloat=0, GLfloat=0, GLfloat=0);
        virtual void fini(GLsizei);

        void proc(GLsizei);

        void uniform(std::string, const double *, bool);
    };

    //-------------------------------------------------------------------------
    // Geometry cache normal generator

    class geonrm : public geogen
    {
    public:

        geonrm(GLsizei, GLsizei);
    };

    //-------------------------------------------------------------------------
    // Geometry cache position generator

    class geopos : public geogen
    {
        GLfloat b[6];
        GLfloat d0;
        GLfloat d1;

    public:

        geopos(GLsizei, GLsizei);

        virtual void init(double, const double *);
        virtual void seed(GLsizei, GLfloat=0, GLfloat=0, GLfloat=0, GLfloat=0);
        virtual void fini(GLsizei);

        double min_d() const { return double(d0); }
        double max_d() const { return double(d1); }
    };

    //-------------------------------------------------------------------------
    // Geometry cache texture coordinate generator

    class geotex : public geogen
    {
    public:

        geotex(GLsizei, GLsizei);
    };

    //-------------------------------------------------------------------------
    // Geometry cache position accumulator

    class geoacc : public geobuf
    {
    public:

        geoacc(GLsizei, GLsizei);

        void init(GLsizei, double, const double *);
    };

    //-------------------------------------------------------------------------
    // Geometry cache position extrema reducer

    class geoext : public geobuf
    {
        ogl::buffer buff;

    public:

        geoext(GLsizei, GLsizei);

        void proc(GLsizei);

        const GLfloat *rmap() const;
        void           umap() const;
    };

    //-------------------------------------------------------------------------
    // Geometry cache vertex buffer

    class geovtx : public ogl::buffer
    {
        GLsizei w;
        GLsizei h;

    public:

        geovtx(GLsizei, GLsizei);

        void read_v(GLsizei);
        void read_n(GLsizei);
        void read_c(GLsizei);
    };

    //-------------------------------------------------------------------------

}

#endif
