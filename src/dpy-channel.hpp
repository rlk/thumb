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

#ifndef DPY_CHANNEL_HPP
#define DPY_CHANNEL_HPP

#include "app-serial.hpp"

//-----------------------------------------------------------------------------
// Forward declarations

namespace ogl
{
    class frame;
}

//-----------------------------------------------------------------------------

namespace dpy
{
    class channel
    {
        double  v[3];      // View position, in head coordinates
        double  p[3];      // View position, in user coordinates (cached)
        GLubyte c[4];      // Calibration target color

        int w;
        int h;

        ogl::frame *back;  // Off-screen render target

    public:

        channel(app::node);
       ~channel();

        void set_head(const double *,
                      const double *);

        void init();
        void fini();
        void bind() const;
        void free() const;
        void test() const;

        int get_w() const { return w; }
        int get_h() const { return h; }

        const double *get_p() const { return p; }

        void bind_color(GLenum t) const { if (back) back->bind_color(t); }
        void free_color(GLenum t) const { if (back) back->free_color(t); }
    };

    typedef std::vector<channel *>           channel_v;
    typedef std::vector<channel *>::iterator channel_i;
}

//-----------------------------------------------------------------------------

#endif
