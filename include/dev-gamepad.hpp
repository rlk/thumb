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

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class gamepad : public input
    {
    public:

        gamepad();

        bool process_event(app::event *);

    private:

        double deaden(double) const;

        // Configuration

        const double dead_zone;
        const double filter;

        const int axis_move_R;
        const int axis_move_L;
        const int axis_move_U;
        const int axis_move_D;
        const int axis_move_B;
        const int axis_move_F;
        const int axis_turn_R;
        const int axis_turn_L;
        const int axis_turn_U;
        const int axis_turn_D;

        // Current state

        double    value_move_R;
        double    value_move_L;
        double    value_move_U;
        double    value_move_D;
        double    value_move_B;
        double    value_move_F;
        double    value_turn_R;
        double    value_turn_L;
        double    value_turn_U;
        double    value_turn_D;

        double    dx;
        double    dy;
        double    dz;
        double    dyaw;
        double    dpitch;

        // Event handlers

        bool process_axis(app::event *);
        bool process_tick(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
