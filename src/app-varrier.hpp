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

#ifndef VARRIER_HPP
#define VARRIER_HPP

#include "app-disp.hpp"
#include "ogl-program.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class varrier : public disp
    {
        frustum *frustL;
        frustum *frustR;

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

        void bind_transform(GLenum, const frustum *);
        void free_transform(GLenum);

    public:

        varrier(app::node, app::node, const int *);
        
        bool input_keybd(int, int, int, bool);

        virtual bool pick(double *, double *, int, int);
        virtual void prep(view_v&, frustum_v&);
        virtual void draw(view_v&, int&, bool);

        virtual ~varrier();
    };
}

//-----------------------------------------------------------------------------

#endif
