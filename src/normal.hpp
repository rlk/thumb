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

#ifndef NORMAL_HPP
#define NORMAL_HPP

#include "disp.hpp"
#include "program.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class normal : public disp
    {
        int      index;
        frustum *frust;

        int x;
        int y;
        int w;
        int h;

        const ogl::program *P;

    public:

        normal(app::node, app::node, const int *);

        virtual void prep(view_v&, frustum_v&);
        virtual void draw(view_v&, int&, bool);

        virtual ~normal();
    };
}

//-----------------------------------------------------------------------------

#endif
