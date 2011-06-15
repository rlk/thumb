//  Copyright (C) 2007-2011 Robert Kooima
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

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class gamepad : public input
    {
        // Configuration

        int gamepad_axis_X;
        int gamepad_axis_Y;
        int gamepad_axis_Z;
        int gamepad_axis_T;

        int gamepad_butn_L;
        int gamepad_butn_R;
        int gamepad_butn_D;
        int gamepad_butn_U;
        int gamepad_butn_F;
        int gamepad_butn_B;
        int gamepad_butn_H;

        double gamepad_axis_X_min;
        double gamepad_axis_X_max;
        double gamepad_axis_Y_min;
        double gamepad_axis_Y_max;
        double gamepad_axis_Z_min;
        double gamepad_axis_Z_max;
        double gamepad_axis_T_min;
        double gamepad_axis_T_max;

        double calibrate(double, double, double);

        // Navigation state

        std::vector<bool> button;

        int    motion[3];
        double rotate[4];

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_axis(app::event *);
        bool process_tick(app::event *);

    public:

        gamepad();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
