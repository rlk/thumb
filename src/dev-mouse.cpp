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

#include <cassert>

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "matrix.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "app-event.hpp"
#include "dev-mouse.hpp"

//-----------------------------------------------------------------------------

dev::mouse::mouse() :
    dragging(false),
    modifier(0)
{
    key_move_L = conf->get_i("key_move_L");
    key_move_R = conf->get_i("key_move_R");
    key_move_D = conf->get_i("key_move_D");
    key_move_U = conf->get_i("key_move_U");
    key_move_F = conf->get_i("key_move_F");
    key_move_B = conf->get_i("key_move_B");

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;

    load_idt(init_R);
    load_idt(curr_R);
}

dev::mouse::~mouse()
{
}

//-----------------------------------------------------------------------------

#ifdef CONFIG_UNCONSTRAINED_TUMBLE

bool dev::mouse::process_point(app::event *E)
{
    load_mat(init_R, curr_R);    

    set_quaternion(curr_R, E->data.point.q);

    if (dragging)
    {
        if (modifier & KMOD_SHIFT)
        {
            double a0 = DEG(atan2(init_R[9], init_R[8]));
            double a1 = DEG(atan2(curr_R[9], curr_R[8]));

            ::user->turn(0.0, 0.0, a0 - a1);
        }
        else
        {
            ::user->tumble(init_R, curr_R);
        }

        ::host->post_draw();
        return true;
    }
    return false;
}

#else

bool dev::mouse::process_point(app::event *E)
{
    load_mat(init_R, curr_R);

    set_quaternion(curr_R, E->data.point.q);

    if (dragging)
    {
        double p0 = DEG(atan2(init_R[9], init_R[10]));
        double p1 = DEG(atan2(curr_R[9], curr_R[10]));

        double t0 = DEG(atan2(init_R[8], init_R[10]));
        double t1 = DEG(atan2(curr_R[8], curr_R[10]));

        ::user->look((t1 - t0) * view_turn_rate,
                     (p0 - p1) * view_turn_rate);

        ::host->post_draw();
        return true;
    }
    return false;
}

#endif

bool dev::mouse::process_click(app::event *E)
{
    const bool d = E->data.click.d;
    const int  b = E->data.click.b;
    const int  m = E->data.click.m;

//  double now = universe.get_time();

    // Handle rotating the view.

    if (b == SDL_BUTTON_RIGHT)
    {
        dragging = d;
        modifier = m;
        return true;
    }

    // Handle rotating the universe.

/* TODO: convert this into a script event.

    else if (d && b == SDL_BUTTON_WHEELUP)
    {
        universe.set_time(now + 15.0 * 60.0);
        return true;
    }
    else if (d && b == SDL_BUTTON_WHEELDOWN)
    {
        universe.set_time(now - 15.0 * 60.0);
        return true;
    }
*/
    return false;
}

bool dev::mouse::process_keybd(app::event *E)
{
    const bool d = E->data.keybd.d;
    const int  k = E->data.keybd.k;
    const int  m = E->data.keybd.m;

    int dd = d ? +1 : -1;

    modifier = m;

    // Handle motion keys.

    if      (k == key_move_L) { motion[0] -= dd; return true; }
    else if (k == key_move_R) { motion[0] += dd; return true; }
    else if (k == key_move_D) { motion[1] -= dd; return true; }
    else if (k == key_move_U) { motion[1] += dd; return true; }
    else if (k == key_move_F) { motion[2] -= dd; return true; }
    else if (k == key_move_B) { motion[2] += dd; return true; }
    
    // Teleport home.

    else if (d)
    {
        if (k == SDLK_RETURN)
        {
            ::user->home();
            ::host->post_draw();
            return true;
        }
    }

    return false;
}

bool dev::mouse::process_timer(app::event *E)
{
    double kp = E->data.timer.dt * view_move_rate * 0.001;;

    if (modifier & KMOD_SHIFT) kp *= 10.0;
    if (modifier & KMOD_CTRL)  kp *=  0.1;

    user->move(motion[0] * kp,
               motion[1] * kp,
               motion[2] * kp);

    return false;
}

bool dev::mouse::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_POINT: R |= process_point(E); break;
    case E_CLICK: R |= process_click(E); break;
    case E_KEYBD: R |= process_keybd(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
