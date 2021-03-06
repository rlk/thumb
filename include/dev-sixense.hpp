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

#ifdef CONFIG_SIXENSE
#ifndef DEV_SIXENSE_HPP
#define DEV_SIXENSE_HPP

#include <dev-input.hpp>
#include <etc-vector.hpp>

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

        vec3 init_p;
        vec3 curr_p;
        quat init_q;
        quat curr_q;

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
#endif
