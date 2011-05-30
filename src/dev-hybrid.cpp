//  Copyright (C) 2010-2011 Robert Kooima
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

#include <SDL_joystick.h>

#include <etc-math.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-hybrid.hpp>

//-----------------------------------------------------------------------------

dev::axis::axis(app::node node, int p, int n, int m) : pval(0), nval(0)
{
    if (node)
    {
        paxis  = node.get_i("paxis", p);
        naxis  = node.get_i("naxis", n);
        mod    = node.get_i("mod",   m);
    }
    else
    {
        paxis = p;
        naxis = n;
        mod   = m;
    }
    active = (mod < 0);
}

void dev::axis::click(int b, bool d)
{
    if (mod >= 0 && mod == b)
        active = d;
}

double dev::axis::value(int a, int d)
{
    if (active)
    {
        if (paxis >= 0 && paxis == a) pval = d;
        if (naxis >= 0 && naxis == a) nval = d;

        return (pval - nval) / 32767.0;
    }
    return 0.0;
}

//-----------------------------------------------------------------------------

dev::button::button(app::node node, int i, int m)
{
    if (node)
    {
        index  = node.get_i("index", i);
        mod    = node.get_i("mod",   m);
    }
    else
    {
        index = i;
        mod   = m;
    }
    active = (mod < 0);
}

bool dev::button::click(int b, bool d)
{
    if (mod >= 0 && mod == b)
        active = d;

    return (active && index == b);
}

//-----------------------------------------------------------------------------

// Default controls conform to the XBox 360 Wireless Receiver.

dev::hybrid::hybrid(const std::string& filename) :

    file(filename),

    node(file.get_root().find("gamepad", "name",
                              SDL_JoystickName(::conf->get_i("gamepad_device")))),

    move_LR  (node.find("axis",   "name", "move_LR"),    0, -1, -1),
    move_FB  (node.find("axis",   "name", "move_FB"),    1, -1, -1),
    move_UD  (node.find("axis",   "name", "move_UD"),    5,  2, -1),

    look_LR  (node.find("axis",   "name", "look_LR"),   -1,  3, -1),
    look_UD  (node.find("axis",   "name", "look_UD"),   -1,  4, -1),

    hand_LR  (node.find("axis",   "name", "hand_LR"),    0, -1,  5),
    hand_FB  (node.find("axis",   "name", "hand_FB"),    1, -1,  5),
    hand_UD  (node.find("axis",   "name", "hand_UD"),    5,  2,  5),

    use      (node.find("button", "name", "use"),        0,     -1),
    move_home(node.find("button", "name", "move_home"), 14,     -1),
    hand_home(node.find("button", "name", "hand_home"), 14,      5),
    peek_U   (node.find("button", "name", "peek_U"),    10,     -1),
    peek_D   (node.find("button", "name", "peek_D"),    11,     -1),
    peek_L   (node.find("button", "name", "peek_L"),    12,     -1),
    peek_R   (node.find("button", "name", "peek_R"),    13,     -1)
{
    position[0] = 0;
    position[1] = 0;
    position[2] = 0;
    rotation[0] = 0;
    rotation[1] = 0;
}

//-----------------------------------------------------------------------------

bool dev::hybrid::process_click(app::event *E)
{
    const int  b = E->data.click.b;
    const bool d = E->data.click.d;

    const double U[3] = { 0, -.05, 0 };
    const double D[3] = { 0, +.05, 0 };
    const double L[3] = { -.05, 0, 0 };
    const double R[3] = { +.05, 0, 0 };
    const double C[3] = { 0, 0, 0    };
    const double q[4] = { 0, 0, 0, 1 };

    bool r = false;

    // Filter this button press through all button observers.

    if (move_home.click(b, d)) { ::user->home(); r = true; }

    if (peek_U.click(b, d)) { ::host->set_head(d ? U : C, q); r = true; }
    if (peek_D.click(b, d)) { ::host->set_head(d ? D : C, q); r = true; }
    if (peek_L.click(b, d)) { ::host->set_head(d ? L : C, q); r = true; }
    if (peek_R.click(b, d)) { ::host->set_head(d ? R : C, q); r = true; }

    // Filter this button press through all value observers.

    move_LR.click(b, d);
    move_FB.click(b, d);
    move_UD.click(b, d);
    look_LR.click(b, d);
    look_UD.click(b, d);
    hand_LR.click(b, d);
    hand_FB.click(b, d);
    hand_UD.click(b, d);
    
    return false;
}

bool dev::hybrid::process_axis(app::event *E)
{
    const int a = E->data.axis.a;
    const int v = E->data.axis.v;

    position[0] = move_LR.value(a, v);
    position[1] = move_UD.value(a, v);
    position[2] = move_FB.value(a, v);
    rotation[0] = look_LR.value(a, v);
    rotation[1] = look_UD.value(a, v);

    return false;
}

bool dev::hybrid::process_timer(app::event *E)
{
    const double dt = E->data.timer.dt * 0.001;
    const double dz = 0.25;

    const double kp = dt * ::user->get_move_rate();
    const double kr = dt * ::user->get_turn_rate();
    const double kt = 3.0 * 60.0 * 60.0 * dt;

    if (fabs(position[0]) > dz ||
        fabs(position[1]) > dz ||
        fabs(position[2]) > dz) ::user->move(position[0] * kp,
                                             position[1] * kp,
                                             position[2] * kp);

    if (fabs(rotation[0]) > dz ||
        fabs(rotation[1]) > dz) ::user->look(rotation[0] * kr,
                                             rotation[1] * kr);

    if (0)
        ::user->pass(kt);

    return false;
}

bool dev::hybrid::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_CLICK: R |= process_click(E); break;
    case E_AXIS: R |= process_axis(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
