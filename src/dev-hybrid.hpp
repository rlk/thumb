//  Copyright (C) 2010 Robert Kooima
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

#ifndef DEV_HYBRID
#define DEV_HYBRID

#include "dev-input.hpp"

//-----------------------------------------------------------------------------

namespace dev
{
    class hybrid : public input
    {
        // Configuration

        int    pos_axis_LR;
        int    pos_axis_FB;
        int    rot_axis_LR;
        int    rot_axis_UD;

        int    button_A;
        int    button_H;

        // Navigation state

        double position[3];
        double rotation[2];

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_value(app::event *);
        bool process_timer(app::event *);

    public:

        hybrid();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
