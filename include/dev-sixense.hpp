//  Copyright (C) 2013 Robert Kooima
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

#ifndef DEV_SIXENSE
#define DEV_SIXENSE

#include <dev-input.hpp>

#include <sixense.h>

//-----------------------------------------------------------------------------

namespace dev
{
    class sixense : public input
    {
        // Configuration

        double move_rate;
        double turn_rate;

        int hand_controller;
        int fly_button;

        // Navigation state

        bool status;
        bool flying;

        double init_p[3], init_q[4];
        double curr_p[3], curr_q[4];

        sixenseAllControllerData prev;

        // Event handlers

        bool process_point (app::event *);
        bool process_axis  (app::event *);
        bool process_button(app::event *);
        bool process_tick  (app::event *);

        void translate();

    public:

        sixense();
       ~sixense();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
