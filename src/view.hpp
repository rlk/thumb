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

#ifndef VIEW_HPP
#define VIEW_HPP

#include "serial.hpp"
#include "frame.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class view
    {
        double  v[3];
        double  p[3];
        GLubyte c[4];

        int w;
        int h;

        ogl::frame *back;

    public:

        view(app::node, const int *);
       ~view();

        void set_head(const double *,
                      const double *);

        const double  *get_p() const { return p; }
        const GLubyte *get_c() const { return c; }

        void bind();
        void free();

        void bind_color(GLenum t) const { if (back) back->bind_color(t); }
        void free_color(GLenum t) const { if (back) back->free_color(t); }
    };

    typedef std::vector<view *>           view_v;
    typedef std::vector<view *>::iterator view_i;
}

//-----------------------------------------------------------------------------

#endif
