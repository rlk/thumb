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

#ifndef DEV_GAMEPAD
#define DEV_GAMEPAD

#include <vector>

#include "uni-universe.hpp"
#include "dev-input.hpp"

//-----------------------------------------------------------------------------

namespace dev
{
    class gamepad : public input
    {
        uni::universe& universe;

        int gamepad_axis_X;
        int gamepad_axis_Y;
        int gamepad_axis_Z;
        int gamepad_axis_A;
        int gamepad_axis_T;

        int gamepad_butn_L;
        int gamepad_butn_R;
        int gamepad_butn_D;
        int gamepad_butn_U;
        int gamepad_butn_F;
        int gamepad_butn_B;

        double view_move_rate;
        double view_turn_rate;

        std::vector<bool> button;

        int    motion[3];
        double rotate[5];

    public:

        gamepad(uni::universe&);
       ~gamepad();

        virtual bool click(int, int, int, bool);
        virtual bool value(int, int, double);
        virtual bool timer(int);
    };
}

//-----------------------------------------------------------------------------

#endif
