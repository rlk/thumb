//  Copyright (C) 2007-2011, 2017 Robert Kooima
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

#ifndef DEV_TRACKD_HPP
#define DEV_TRACKD_HPP

#include <dev-input.hpp>
#include <etc-vector.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class trackd : public input
    {
        // Configuration

	double scale;
        double move_rate;
        double turn_rate;

        int head_sensor;
        int hand_sensor;

        int fly_button;

        // Navigation state

        bool status;
        bool flying;

        vec3 curr_head_p;
        quat curr_head_q;

        vec3 curr_hand_p;
        quat curr_hand_q;

        vec3 init_hand_p;
        quat init_hand_q;

        // Event handlers

        bool process_point (app::event *);
        bool process_axis  (app::event *);
        bool process_button(app::event *);
        bool process_tick  (app::event *);

        void translate();

    public:

        trackd();
       ~trackd();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
