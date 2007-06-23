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

#include "world.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class mode
    {
        int  drag_x;
        int  drag_y;
        bool drag_d;

    protected:

        wrl::world& world;

    public:

        mode(wrl::world& w) : drag_d(false), world(w) { }

        virtual void enter() { }
        virtual void leave() { }

        virtual bool point(const float[3], const float[3], int, int);
        virtual bool click(int, bool);
        virtual bool keybd(int, bool, int);
        virtual bool timer(float);

        virtual GLfloat view(const GLfloat *);
        virtual void    draw(const GLfloat *);

        virtual ~mode() { }
    };
}

//-----------------------------------------------------------------------------

#endif
