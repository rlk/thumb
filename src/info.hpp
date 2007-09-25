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

#ifndef INFO_HPP
#define INFO_HPP

#include "control.hpp"
#include "mode.hpp"

//-----------------------------------------------------------------------------

namespace mode
{
    class info : public mode
    {
        cnt::control *gui;

    public:

        info(wrl::world&);

        virtual void enter();
        virtual void leave();

        virtual bool point(const double *, const double *);
        virtual bool click(int, bool);
        virtual bool keybd(int, bool, int);
        virtual bool timer(double);

        virtual double view(const double *);
        virtual void   draw(const double *);
    };
}

//-----------------------------------------------------------------------------

#endif
