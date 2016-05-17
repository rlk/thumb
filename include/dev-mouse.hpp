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

#ifndef DEV_MOUSE
#define DEV_MOUSE

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class mouse : public input
    {
    public:

        mouse();

        virtual bool process_event(app::event *);

        virtual ~mouse();

    private:

        // Configuration

        int key_move_L;
        int key_move_R;
        int key_move_D;
        int key_move_U;
        int key_move_F;
        int key_move_B;

        int key_turn_L;
        int key_turn_R;
        int key_turn_D;
        int key_turn_U;

        double filter;
        double speed;
        int    mode;

        // Navigation state

        bool dragging;
        int  modifier;

        int move_L;
        int move_R;
        int move_D;
        int move_U;
        int move_F;
        int move_B;

        int turn_L;
        int turn_R;
        int turn_D;
        int turn_U;

        quat last_q;
        quat curr_q;

        // Filtered differentials.

        vec3      dpos;
        double    dyaw;
        double    dpitch;

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_tick (app::event *);
        bool process_key  (app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
