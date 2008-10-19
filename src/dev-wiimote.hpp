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

#ifndef DEV_WIIMOTE
#define DEV_WIIMOTE

#include "uni-universe.hpp"
#include "dev-input.hpp"

//-----------------------------------------------------------------------------

namespace dev
{
#ifdef ENABLE_WIIMOTE

    class wiimote : public input
    {
        uni::universe& universe;

        double view_move_rate;
        double view_turn_rate;

        double move[3];
        double turn[5];

        void translate(int t);

    public:

        wiimote(uni::universe&);

        virtual bool point(int, const double *, const double *);
        virtual bool click(int, int, int, bool);
        virtual bool value(int, int, double);
        virtual bool timer(int);

        virtual ~wiimote();
    };

#else  // ENABLE_WIIMOTE

    class wiimote : public input
    {
    public:

        wiimote(uni::universe&) { }
        virtual ~wiimote()      { }
    };

#endif // ENABLE_WIIMOTE
}

//-----------------------------------------------------------------------------

#endif
