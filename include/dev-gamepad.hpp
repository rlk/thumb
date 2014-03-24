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
        static const int naxis =  6;
        static const int nbutn = 16;

        // Configuration

        int gamepad_axis[naxis];
        int gamepad_butn[nbutn];

        // Current state

        double axis[naxis];
        bool   butn[nbutn];

        // Event handlers

        bool process_button(app::event *);
        bool process_axis  (app::event *);
        bool process_tick  (app::event *);

    public:

        gamepad();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
