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

#ifndef GEOREN_HPP
#define GEOREN_HPP

#include <string>

#include "opengl.hpp"
#include "program.hpp"
#include "frame.hpp"

namespace uni
{
    //-------------------------------------------------------------------------
    // Geometry render buffer

    class renbuf : public ogl::frame
    {
    protected:

        const ogl::program *draw;

    public:

        renbuf(GLsizei, GLsizei, GLenum, bool, bool, std::string, std::string);
       ~renbuf();

        virtual void bind(bool=false) const;
        virtual void free(bool=false) const;
    };

    //-------------------------------------------------------------------------
    // Geometry texture coordinate render target

    class cylbuf : public renbuf
    {
    public:
        cylbuf(GLsizei, GLsizei);
    };

    //-------------------------------------------------------------------------
    // Geometry diffuse render target

    class difbuf : public renbuf
    {
        cylbuf& cyl;

    public:
        difbuf(GLsizei, GLsizei, cylbuf&);

        void bind(bool=false) const;
        void free(bool=false) const;
    };

    //-------------------------------------------------------------------------
    // Geometry normal render target

    class nrmbuf : public renbuf
    {
        cylbuf& cyl;

    public:
        nrmbuf(GLsizei, GLsizei, cylbuf&);

        void axis(const double *) const;

        void bind(bool=false) const;
        void free(bool=false) const;
    };

    //-------------------------------------------------------------------------
    // Geometry render manager

    class georen
    {
        GLsizei w;
        GLsizei h;

        const ogl::program *draw;

        cylbuf _cyl;
        difbuf _dif;
        nrmbuf _nrm;

    public:

        georen(GLsizei, GLsizei);
       ~georen();

        const cylbuf *cyl() const { return &_cyl; }
        const difbuf *dif() const { return &_dif; }
        const nrmbuf *nrm() const { return &_nrm; }

        void bind() const;
        void free() const;
    };


    //-------------------------------------------------------------------------
}

#endif
