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
#include "world.hpp"
#include "mode.hpp"
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

    // Entity state.

    wrl::world world;

    // Editor mode.

    mode::mode *edit;
    mode::mode *play;
    mode::mode *info;
    mode::mode *curr;

    void goto_mode(mode::mode *);

    // Demo state.

    int last_x;
    int last_y;
    int button[4];
    int motion[3];

    void draw_all();

public:

    demo();
   ~demo();

    void point(int, int);
    void click(int, bool);
    void keybd(int, bool, int);
    void timer(double);

    void draw();
};

//-----------------------------------------------------------------------------

#endif
