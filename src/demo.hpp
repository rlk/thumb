//  Copyright (C) 2005 Robert Kooima
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

#ifndef DEMO_HPP
#define DEMO_HPP

#include <set>

#include "opengl.hpp"
#include "universe.hpp"
#include "world.hpp"
#include "mode.hpp"
#include "glob.hpp"
#include "prog.hpp"

//-----------------------------------------------------------------------------

class demo : public app::prog
{
    // Configuration.

    int key_edit;
    int key_play;
    int key_info;

    int key_move_L;
    int key_move_R;
    int key_move_F;
    int key_move_B;

    double view_move_rate;
    double view_turn_rate;

    int tracker_head_sensor;
    int tracker_hand_sensor;

    // Entity state.

    uni::universe universe;
    wrl::world    world;

    // Editor mode.

    mode::mode *edit;
    mode::mode *play;
    mode::mode *info;
    mode::mode *curr;

    void goto_mode(mode::mode *);

    // Demo state.

    int button[4];

    bool   attr_mode;
    bool   attr_stop;
    double attr_curr;
    double attr_time;

    void attr_on();
    void attr_off();
    void attr_next();
    void attr_prev();

    // Tracker cache.

    double init_P[3], init_R[16];
    double curr_P[3], curr_R[16];

    bool draw_sphere;

public:

    demo();
   ~demo();

    void point(int, const double *, const double *);
    void click(int, int, int, bool);
    void keybd(int, int, int, bool);
    void value(int, int, double);
    void timer(int);

    void prep(app::frustum_v&);
    void draw(int);
};

//-----------------------------------------------------------------------------

#endif
