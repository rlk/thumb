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

#ifndef DPY_VARRIER_HPP
#define DPY_VARRIER_HPP

#include "dpy-display.hpp"
#include "ogl-program.hpp"
#include "app-file.hpp"

//-----------------------------------------------------------------------------

namespace dpy
{
    class varrier : public display
    {
        app::frustum *frustL;
        app::frustum *frustR;

        int count;
        int x;
        int y;
        int w;
        int h;

        const ogl::program *P;

        double pitch;
        double angle;
        double thick;
        double shift;
        double cycle;

        double pix;

        app::node node;

        void bind_transform(GLenum, const app::frustum *);
        void free_transform(GLenum);

    public:

        varrier(app::node, app::node, const int *);

        // Calibration handlers.
        
        virtual bool input_keybd(int, int, int, bool);

        // Rendering handlers.

        virtual bool pick(double *, double *, int, int);
        virtual void prep(app::view_v&, app::frustum_v&);
        virtual void draw(app::view_v&, int&, bool, bool);

        virtual ~varrier();
    };
}

//-----------------------------------------------------------------------------

#endif
