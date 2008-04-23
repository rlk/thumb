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

#ifndef DEV_INPUT
#define DEV_INPUT

//-----------------------------------------------------------------------------

namespace dev
{
    class input
    {
    public:

        virtual bool point(int, const double *, const double *);
        virtual bool click(int, int, int, bool);
        virtual bool keybd(int, int, int, bool);
        virtual bool value(int, int, double);
        virtual bool timer(int);
    };
}

//-----------------------------------------------------------------------------

#endif
