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

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "matrix.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "dev-mouse.hpp"

//-----------------------------------------------------------------------------

dev::mouse::mouse(uni::universe& universe) :
    universe(universe),
    modifiers(0)
{
    key_move_L = conf->get_i("key_move_L");
    key_move_R = conf->get_i("key_move_R");
    key_move_D = conf->get_i("key_move_D");
    key_move_U = conf->get_i("key_move_U");
    key_move_F = conf->get_i("key_move_F");
    key_move_B = conf->get_i("key_move_B");

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    init_P[0] = curr_P[0] = 0;
    init_P[1] = curr_P[1] = 0;
    init_P[2] = curr_P[2] = 0;

    load_idt(init_R);
    load_idt(curr_R);

    button.reserve(5);
}

dev::mouse::~mouse()
{
}

//-----------------------------------------------------------------------------

bool dev::mouse::point(int i, const double *p, const double *q)
{
    load_mat(init_R, curr_R);    
    set_quaternion(curr_R, q);

    if (button[SDL_BUTTON_LEFT])
    {
        if (modifiers & KMOD_SHIFT)
        {
            double a0 = DEG(atan2(init_R[9], init_R[8]));
            double a1 = DEG(atan2(curr_R[9], curr_R[8]));

            ::user->turn(0.0, 0.0, a0 - a1);
        }
        else
        {
            ::user->tumble(init_R, curr_R);
        }
        return true;
    }
    return false;
}

bool dev::mouse::click(int i, int b, int m, bool d)
{
    button[b] = d;
    modifiers = m;

    if      (d && b == SDL_BUTTON_WHEELUP)
    {
        if (m & KMOD_SHIFT)
            universe.turn( 0.0, -1.0);
        else
            universe.turn(+1.0,  0.0);
    }
    else if (d && b == SDL_BUTTON_WHEELDOWN)
    {
        if (m & KMOD_SHIFT)
            universe.turn( 0.0, +1.0);
        else
            universe.turn(-1.0,  0.0);
    }

    return true;
}

bool dev::mouse::keybd(int c, int k, int m, bool d)
{
    int dd = d ? +1 : -1;

    modifiers = m;

    if      (k == key_move_L) { curr_P[0] -= dd; return true; }
    else if (k == key_move_R) { curr_P[0] += dd; return true; }
    else if (k == key_move_D) { curr_P[1] -= dd; return true; }
    else if (k == key_move_U) { curr_P[1] += dd; return true; }
    else if (k == key_move_F) { curr_P[2] -= dd; return true; }
    else if (k == key_move_B) { curr_P[2] += dd; return true; }
    
    else if (d)
    {
        if (k == SDLK_HOME) { ::user->home(); return true; }
    }

    return false;
}

bool dev::mouse::timer(int t)
{
    double dt = t / 1000.0;

    double dP[3], kp = dt * universe.rate();

    dP[0] = curr_P[0] - init_P[0];
    dP[1] = curr_P[1] - init_P[1];
    dP[2] = curr_P[2] - init_P[2];

    user->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);

    if (DOT3(dP, dP) > 0.1)
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------
