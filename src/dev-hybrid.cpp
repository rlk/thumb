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
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-hybrid.hpp>

//-----------------------------------------------------------------------------

dev::axis::axis(app::node node, int p, int n, int u, int d) : pval(0), nval(0)
{
    if (node)
    {
        paxis  = node.get_i("paxis", p);
        naxis  = node.get_i("naxis", n);
        up     = node.get_i("up",    u);
        dn     = node.get_i("dn",    d);
    }
    else
    {
        paxis = p;
        naxis = n;
        up    = u;
        dn    = d;
    }
    if (paxis >= 0 && naxis >= 0)
    {
        pval = -32767;
        nval = -32767;
    }
    dnok = (dn < 0);
    upok = true;
}

void dev::axis::process_button(int b, bool d)
{
    if (up == b) upok = !d;
    if (dn == b) dnok =  d;
}

double dev::axis::process_axis(int a, int d)
{
    if (upok && dnok)
    {
        if (paxis >= 0 && paxis == a) pval = d;
        if (naxis >= 0 && naxis == a) nval = d;

        return (pval - nval) / 32767.0;
    }
    return 0.0;
}

//-----------------------------------------------------------------------------

dev::button::button(app::node node, int i, int u, int d)
{
    if (node)
    {
        index  = node.get_i("index", i);
        up     = node.get_i("up",    u);
        dn     = node.get_i("dn",    d);
    }
    else
    {
        index = i;
        up    = u;
        dn    = d;
    }
    dnok = (dn < 0);
    upok = true;
}

bool dev::button::process_button(int b, bool d)
{
    if (up == b) upok = !d;
    if (dn == b) dnok =  d;

    return (upok && dnok && index == b);
}

//-----------------------------------------------------------------------------

void dev::hybrid::recenter_hand()
{
    hand_q[0]   =  0.0;
    hand_q[1]   =  0.0;
    hand_q[2]   =  0.0;
    hand_q[3]   =  1.0;
    hand_p[0]   =  0.0;
    hand_p[1]   = -0.3;
    hand_p[2]   = -1.0;
}

const char *joystick()
{
    const char *s = SDL_JoystickName(::conf->get_i("gamepad_device"));
    return s ? s : "default";
}

// Default controls conform to the XBox 360 Wireless Receiver.

dev::hybrid::hybrid(const std::string& filename) :

    file(filename),

    node(file.get_root().find("gamepad", "name", joystick())),

    look_LR  (node.find("axis",   "name", "look_LR"),   -1,  3),
    look_UD  (node.find("axis",   "name", "look_UD"),   -1,  4),

    move_LR  (node.find("axis",   "name", "move_LR"),    0, -1,  5, -1),
    move_FB  (node.find("axis",   "name", "move_FB"),    1, -1,  5, -1),
    move_UD  (node.find("axis",   "name", "move_UD"),    5,  2,  5, -1),

    hand_LR  (node.find("axis",   "name", "hand_LR"),    0, -1, -1,  5),
    hand_FB  (node.find("axis",   "name", "hand_FB"),    1, -1, -1,  5),
    hand_UD  (node.find("axis",   "name", "hand_UD"),    5,  2, -1,  5),

    button_0 (node.find("button", "name", "button_0"),   0),
    button_1 (node.find("button", "name", "button_1"),   1),
    button_2 (node.find("button", "name", "button_2"),   2),
    button_3 (node.find("button", "name", "button_3"),   3),
    move_home(node.find("button", "name", "move_home"), 14,  5, -1),
    hand_home(node.find("button", "name", "hand_home"), 14, -1,  5),
    peek_U   (node.find("button", "name", "peek_U"),    10),
    peek_D   (node.find("button", "name", "peek_D"),    11),
    peek_L   (node.find("button", "name", "peek_L"),    12),
    peek_R   (node.find("button", "name", "peek_R"),    13),

    move_rate( 5.0),
    turn_rate(60.0),
    
    depth(0)
{
    position[0] =  0.0;
    position[1] =  0.0;
    position[2] =  0.0;
    rotation[0] =  0.0;
    rotation[1] =  0.0;
    hand_v[0]   =  0.0;
    hand_v[1]   =  0.0;
    hand_v[2]   =  0.0;

    recenter_hand();
}

//-----------------------------------------------------------------------------

void dev::hybrid::synthesize_point()
{
    app::event E;

    if (depth == 0)
    {
        depth++;
        ::host->process_event(E.mk_point(0, hand_p, hand_q));
        depth--;
    }
}

bool dev::hybrid::process_point(app::event *E)
{
    const double *q = E->data.point.q;

    hand_q[0] = q[0];
    hand_q[1] = q[1];
    hand_q[2] = q[2];
    hand_q[3] = q[3];

    synthesize_point();

    return false;
}

bool dev::hybrid::process_button(app::event *E)
{
    const double U[3] = { 0, -.05, 0 };
    const double D[3] = { 0, +.05, 0 };
    const double L[3] = { -.05, 0, 0 };
    const double R[3] = { +.05, 0, 0 };
    const double C[3] = { 0, 0, 0    };
    const double q[4] = { 0, 0, 0, 1 };

    const int  b = E->data.button.b;
    const bool d = E->data.button.d;

    app::event F;

    // Filter this button press (unclaimed) through all axis observers.

    move_LR.process_button(b, d);
    move_FB.process_button(b, d);
    move_UD.process_button(b, d);
    look_LR.process_button(b, d);
    look_UD.process_button(b, d);
    hand_LR.process_button(b, d);
    hand_FB.process_button(b, d);
    hand_UD.process_button(b, d);

    // Check this button press against all user controls.

    if (hand_home.process_button(b, d))
    {
        recenter_hand();
        synthesize_point();
        return true;
    }
    if (move_home.process_button(b, d))
    {
        ::view->go_home();
        synthesize_point();
        return true;
    }
    if (peek_U.process_button(b, d))
    {
        ::host->set_head(d ? U : C, q);
        return true;
    }
    if (peek_D.process_button(b, d))
    {
        ::host->set_head(d ? D : C, q);
        return true;
    }
    if (peek_L.process_button(b, d))
    {
        ::host->set_head(d ? L : C, q);
        return true;
    }
    if (peek_R.process_button(b, d))
    {
        ::host->set_head(d ? R : C, q);
        return true;
    }

    // Translate this button press as necessary.

    if (::host->root())
    {
        if (button_0.process_button(b, d))
            return ::host->process_event(F.mk_click(0, 0, d));

        if (button_1.process_button(b, d))
            return ::host->process_event(F.mk_click(1, 0, d));

        if (button_2.process_button(b, d))
            return ::host->process_event(F.mk_click(2, 0, d));

        if (button_3.process_button(b, d))
            return ::host->process_event(F.mk_click(3, 0, d));
    }
    return false;
}

bool dev::hybrid::process_axis(app::event *E)
{
    // Update all axis observers.

    const int a = int(E->data.axis.a);
    const int v = int(E->data.axis.v);

    position[0] = move_LR.process_axis(a, v);
    position[1] = move_UD.process_axis(a, v);
    position[2] = move_FB.process_axis(a, v);
    rotation[0] = look_LR.process_axis(a, v);
    rotation[1] = look_UD.process_axis(a, v);
    hand_v[0]   = hand_LR.process_axis(a, v);
    hand_v[1]   = hand_UD.process_axis(a, v);
    hand_v[2]   = hand_FB.process_axis(a, v);

    return false;
}

bool dev::hybrid::process_tick(app::event *E)
{
#if 0
    // Apply the current position and rotation velocities to the user.

    const double dt = E->data.tick.dt;
    const double dz = 0.25;

    const double kp = dt * move_rate;
    const double kr = dt * turn_rate;

    if (fabs(position[0]) > dz ||
        fabs(position[1]) > dz ||
        fabs(position[2]) > dz)
    {
        ::view->move(position[0] * kp,
                     position[1] * kp,
                     position[2] * kp);
        synthesize_point();
    }

    if (fabs(rotation[0]) > dz ||
        fabs(rotation[1]) > dz)
    {
        ::view->look(rotation[0] * kr,
                     rotation[1] * kr);
        synthesize_point();
    }

    if (fabs(hand_v[0]) > dz ||
        fabs(hand_v[1]) > dz ||
        fabs(hand_v[2]) > dz)
    {
        hand_p[0] += hand_v[0] * dt;
        hand_p[1] += hand_v[1] * dt;
        hand_p[2] += hand_v[2] * dt;
        synthesize_point();
    }
#endif
    return false;
}

bool dev::hybrid::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_POINT:  R |= process_point (E); break;
    case E_BUTTON: R |= process_button(E); break;
    case E_AXIS:   R |= process_axis  (E); break;
    case E_TICK:   R |= process_tick  (E); break;
    }

    return R || dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
