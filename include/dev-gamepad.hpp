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

        const int axis_pos_x;
        const int axis_neg_x;
        const int axis_pos_y;
        const int axis_neg_y;
        const int axis_pos_z;
        const int axis_neg_z;
        const int axis_pos_yaw;
        const int axis_neg_yaw;
        const int axis_pos_pitch;
        const int axis_neg_pitch;
        const int axis_pos_roll;
        const int axis_neg_roll;

        // Current state

        double    value_pos_x;
        double    value_neg_x;
        double    value_pos_y;
        double    value_neg_y;
        double    value_pos_z;
        double    value_neg_z;
        double    value_pos_yaw;
        double    value_neg_yaw;
        double    value_pos_pitch;
        double    value_neg_pitch;
        double    value_pos_roll;
        double    value_neg_roll;

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
