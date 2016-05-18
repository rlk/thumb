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

#include <SDL_keyboard.h>

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

        typedef std::vector<int> keyset;

        keyset move_L;
        keyset move_R;
        keyset move_D;
        keyset move_U;
        keyset move_F;
        keyset move_B;

        keyset turn_L;
        keyset turn_R;
        keyset turn_D;
        keyset turn_U;

        double filter;
        double speed;
        int    mode;

        // Navigation state

        bool dragging;
        bool modified;

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

        // Keyboard handling

        void parse_keyset(      keyset&, const std::string&, int);
        int  check_keyset(const keyset&) const;

        // Keyboard states are given by SDL, but we must replicate them when
        // performing distributed rendering.

        bool keystate[SDL_NUM_SCANCODES];
    };
}

//-----------------------------------------------------------------------------

#endif
