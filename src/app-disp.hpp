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

#ifndef DISP_HPP
#define DISP_HPP

#include <vector>

#include "app-serial.hpp"
#include "app-frustum.hpp"
#include "app-view.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    // Interface for display handling (Mono/Varrier/Dome/Anaglyph/etc)

    class disp
    {
    public:

        disp() { }

        virtual bool input_point(int, const double *, const double *) {
            return false;
        }
        virtual bool input_click(int, int, int, bool) {
            return false;
        }
        virtual bool input_keybd(int, int, int, bool) {
            return false;
        }

        virtual bool pick(double *, double *, int, int) { return false; }
        virtual void prep(view_v&, frustum_v&) { }
        virtual void draw(view_v&, int&, bool) { }

        virtual ~disp() { }
    };

    typedef std::vector<disp *>           disp_v;
    typedef std::vector<disp *>::iterator disp_i;
}

//-----------------------------------------------------------------------------

#endif
