//  Copyright (C) 2007 Robert Kooima
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

#include "uni-universe.hpp"
#include "dev-input.hpp"

//-----------------------------------------------------------------------------

namespace dev
{
    class tracker : public input
    {
        uni::universe& universe;

        int tracker_head_sensor;
        int tracker_hand_sensor;

        int tracker_butn_fly;
        int tracker_butn_home;

        int tracker_axis_A;
        int tracker_axis_T;

        double view_move_rate;
        double view_turn_rate;

        double rotate[2];

        double init_P[3], init_R[16];
        double curr_P[3], curr_R[16];

        std::vector<bool> button;

        void translate();

    public:

        tracker(uni::universe&);

        virtual bool point(int, const double *, const double *);
        virtual bool click(int, int, int, bool);
        virtual bool value(int, int, double);
        virtual bool timer(int);

        virtual ~tracker();
    };
}

//-----------------------------------------------------------------------------

#endif
