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

#ifndef DEV_MOUSE
#define DEV_MOUSE

#include <vector>

#include "uni-universe.hpp"
#include "dev-input.hpp"

//-----------------------------------------------------------------------------

namespace dev
{
    class mouse : public input
    {
        uni::universe& universe;

        int key_move_L;
        int key_move_R;
        int key_move_D;
        int key_move_U;
        int key_move_F;
        int key_move_B;

        double view_move_rate;
        double view_turn_rate;

        std::vector<bool> button;

        int modifiers;

        double init_P[3], init_R[16];
        double curr_P[3], curr_R[16];

    public:

        mouse(uni::universe&);
       ~mouse();

        virtual bool point(int, const double *, const double *);
        virtual bool click(int, int, int, bool);
        virtual bool keybd(int, int, int, bool);
        virtual bool timer(int);
    };
}

//-----------------------------------------------------------------------------

#endif
