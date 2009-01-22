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

#ifndef ANAGLYPH_HPP
#define ANAGLYPH_HPP

#include "dpy-display.hpp"
#include "ogl-program.hpp"

//-----------------------------------------------------------------------------

namespace dpy
{
    class anaglyph : public display
    {
        app::frustum *frustL;
        app::frustum *frustR;

        int count;
        int x;
        int y;
        int w;
        int h;

        const ogl::program *P;

        app::node node;

        void bind_transform(GLenum, const app::frustum *);
        void free_transform(GLenum);

    public:

        anaglyph(app::node, app::node, const int *);
        
        // Rendering handlers.

        virtual bool pick(double *, double *, int, int);
        virtual void prep(app::view_v&, app::frustum_v&);
        virtual void draw(app::view_v&, int&, bool, bool);

        virtual ~anaglyph();
    };
}

//-----------------------------------------------------------------------------

#endif
