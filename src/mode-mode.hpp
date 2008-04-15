//  Copyright (C) 2005 Robert Kooima
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

#ifndef MODE_HPP
#define MODE_HPP

#include "wrl-world.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class mode
    {
    protected:

        wrl::world& world;

    public:

        mode(wrl::world& w) : world(w) { }

        virtual void enter() { }
        virtual void leave() { }

        virtual bool point(int, const double *, const double *);
        virtual bool click(int, int, int, bool);
        virtual bool keybd(int, int, int, bool);
        virtual bool timer(int);

        virtual double view(const double *);
        virtual void   draw(const double *);

        virtual ~mode() { }
    };
}

//-----------------------------------------------------------------------------

#endif