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

        // Gamepad dead zone and filter coefficients.

        const int    move_mode;
        const double dead_zone;
        const double filter;

        // Gamepad axis for each degree of freedom.

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

        // Gamepad button for each degree of freedom.

        const int button_move_R;
        const int button_move_L;
        const int button_move_U;
        const int button_move_D;
        const int button_move_B;
        const int button_move_F;
        const int button_turn_R;
        const int button_turn_L;
        const int button_turn_U;
        const int button_turn_D;

        // Current value of each gamepad axis.

        double    k_move_R;
        double    k_move_L;
        double    k_move_U;
        double    k_move_D;
        double    k_move_B;
        double    k_move_F;
        double    k_turn_R;
        double    k_turn_L;
        double    k_turn_U;
        double    k_turn_D;

        // Current state of each gamepad button.

        double    b_move_R;
        double    b_move_L;
        double    b_move_U;
        double    b_move_D;
        double    b_move_B;
        double    b_move_F;
        double    b_turn_R;
        double    b_turn_L;
        double    b_turn_U;
        double    b_turn_D;

        // Filtered differentials.

        vec3      dpos;
        double    dyaw;
        double    dpitch;

        // Event handlers

        bool process_button(app::event *);
        bool process_axis  (app::event *);
        bool process_tick  (app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
