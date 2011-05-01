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

#ifndef DEV_TRACKER
#define DEV_TRACKER

#include <vector>

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class tracker : public input
    {
        // Configuration

        double scale;

        int tracker_head_sensor;
        int tracker_hand_sensor;

        int tracker_butn_fly;
        int tracker_butn_home;

        int tracker_axis_A;
        int tracker_axis_T;

        int key_edit;
        int key_play;
        int key_info;

        // Navigation state

        double init_P[3], init_R[16];
        double curr_P[3], curr_R[16];

        bool   flying;
        double joy_x;
        double joy_y;

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_value(app::event *);
        bool process_timer(app::event *);

        void translate() const;

    public:

        tracker();
       ~tracker();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
