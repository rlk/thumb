//  Copyright (C) 2009-2011 Robert Kooima
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

#ifndef OGL_CUBELUT_HPP
#define OGL_CUBELUT_HPP

#include <ogl-process.hpp>

//-----------------------------------------------------------------------------

namespace ogl
{
    class cubelut : public process
    {
    protected:

        int n;

        GLuint object;

        double angle(const double *a,
                     const double *b,
                     const double *c,
                     const double *d) const;

        virtual void fill(float *, const double *,
                                   const double *,
                                   const double *,
                                   const double *) const = 0;
    public:

        cubelut(const std::string&, int);

        virtual void init();
        virtual void fini();

        void bind(GLenum) const;

        virtual ~cubelut() { }
    };
}

//-----------------------------------------------------------------------------

#endif
