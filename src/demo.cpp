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

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <iostream>
#include <cstring>

#include "opengl.hpp"
#include "demo.hpp"
#include "conf.hpp"
#include "user.hpp"
#include "host.hpp"
#include "edit.hpp"
#include "play.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

demo::demo() : draw_sphere(false)
{
    edit = new mode::edit(world);
    play = new mode::play(world);
    info = new mode::info(world);

    // Initialize the demo configuration.

    key_edit   = conf->get_i("key_edit");
    key_play   = conf->get_i("key_play");
    key_info   = conf->get_i("key_info");

    key_move_L = conf->get_i("key_move_L");
    key_move_R = conf->get_i("key_move_R");
    key_move_F = conf->get_i("key_move_F");
    key_move_B = conf->get_i("key_move_B");

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    tracker_head_sensor = conf->get_i("tracker_head_sensor");
    tracker_hand_sensor = conf->get_i("tracker_hand_sensor");

    // Initialize the demo state.

    button[0] = 0;
    button[1] = 0;
    button[2] = 0;
    button[3] = 0;

    init_P[0] = 0;
    init_P[1] = 0;
    init_P[2] = 0;

    curr_P[0] = 0;
    curr_P[1] = 0;
    curr_P[2] = 0;

    load_idt(init_R);
    load_idt(curr_R);

    curr = 0;

    attr_time = conf->get_f("attract_delay");
    attr_curr = 0;
    attr_mode = false;

    goto_mode(play);
}

demo::~demo()
{
    delete info;
    delete play;
    delete edit;
}

//-----------------------------------------------------------------------------

void demo::goto_mode(mode::mode *next)
{
    if (curr) curr->leave();
    if (next) next->enter();

    curr = next;
}

void demo::attr_on()
{
    attr_mode = true;
    attr_stop = false;
    ::user->gonext(5.0);
}

void demo::attr_off()
{
    attr_mode = false;
    attr_curr = 0;
}

void demo::attr_next()
{
    attr_mode = true;
    attr_stop = true;
    ::user->gonext(2.0);
}

void demo::attr_prev()
{
    attr_mode = true;
    attr_stop = true;
    ::user->goprev(2.0);
}

//-----------------------------------------------------------------------------

void demo::point(int i, const double *p, const double *q)
{
/* TODO
    int x = 0;
    int y = 0;

    ::host->gui_pick(x, y, p, v);
*/

    if (curr->point(i, p, q) == false)
    {
        set_quaternion(curr_R, q);

        prog::point(i, p, q);
    }
}

void demo::click(int i, int b, int m, bool d)
{
    attr_off();

    button[b] = d;

    if      (d && b == SDL_BUTTON_WHEELUP)   universe.turn(+1.0);
    else if (d && b == SDL_BUTTON_WHEELDOWN) universe.turn(-1.0);

    else if (d && b == 1)
    {
        memcpy(init_R, curr_R, 16 * sizeof (double));
    }
    else
    {
        if (curr->click(i, b, m, d) == false)
            prog::click(i, b, m, d);
    }
}

void demo::keybd(int c, int k, int m, bool d)
{
    prog::keybd(c, k, m, d);

    // Handle mode transitions.

    if      (d && k == key_edit && curr != edit) goto_mode(edit);
    else if (d && k == key_play && curr != play) goto_mode(play);
    else if (d && k == key_info && curr != info) goto_mode(info);

    // Let the current mode take it.

    else if (curr->keybd(c, k, m, d) == false)
    {
        int dd = d ? +1 : -1;

        // Handle view motion keys.

        if      (k == key_move_L) { curr_P[0] -= dd; attr_off(); }
        else if (k == key_move_R) { curr_P[0] += dd; attr_off(); }
        else if (k == key_move_F) { curr_P[2] -= dd; attr_off(); }
        else if (k == key_move_B) { curr_P[2] += dd; attr_off(); }

        else if (d)
        {
            if (k == SDLK_HOME) ::user->home();

            // Handle guided view keys.

            if (m & KMOD_SHIFT)
            {
                if      (k == SDLK_PAGEUP)   attr_next();
                else if (k == SDLK_PAGEDOWN) attr_prev();
                else if (k == SDLK_INSERT)   ::user->insert(universe.get_a());
                else if (k == SDLK_DELETE)   ::user->remove();
                else if (k == SDLK_SPACE)    attr_on();
            }
        }
    }
}

void demo::timer(int t)
{
    double dt = t / 1000.0;

    if (attr_mode)
    {
        double a = 0.0;

        if (user->dostep(dt, universe.get_p(), a))
        {
            if (attr_stop)
                attr_off();
            else
                user->gonext(10.0);
        }
        universe.set_a(a);
    }
    else
    {
        if (button[1])
        {
            // Handle navigation.

            double kp = dt * universe.rate();
            double kr = dt * view_turn_rate;

            double dP[3];
            double dR[3];
            double dz[3];
            double dy[3];

            dP[0] = curr_P[ 0] - init_P[ 0];
            dP[1] = curr_P[ 1] - init_P[ 1];
            dP[2] = curr_P[ 2] - init_P[ 2];

            dy[0] = init_R[ 4] - curr_R[ 4];
            dy[1] = init_R[ 5] - curr_R[ 5];
            dy[2] = init_R[ 6] - curr_R[ 6];

            dz[0] = init_R[ 8] - curr_R[ 8];
            dz[1] = init_R[ 9] - curr_R[ 9];
            dz[2] = init_R[10] - curr_R[10];

            dR[0] =  DOT3(dz, init_R + 4);
            dR[1] = -DOT3(dz, init_R + 0);
            dR[2] =  DOT3(dy, init_R + 0);

            user->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
            user->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
        }
        else
        {
            // Handle auto-attract mode.

            attr_curr += dt;

            if (attr_curr > attr_time)
                attr_on();
        }
    }

    curr->timer(t);
    prog::timer(t);
}

void demo::value(int d, int a, double v)
{
    if (fabs(v) > 0.1)
    {
        attr_off();

        if (a == 0) universe.turn(v);
    }
}

//-----------------------------------------------------------------------------

void demo::prep(app::frustum_v& frusta)
{
    universe.prep(frusta);
}

void demo::draw(int i)
{
/*
    GLfloat A[4] = { 0.45f, 0.50f, 0.55f, 0.0f };
    GLfloat D[4] = { 1.00f, 1.00f, 0.90f, 0.0f };
*/

    GLfloat A[4] = { 0.25f, 0.25f, 0.25f, 0.0f };
    GLfloat D[4] = { 1.00f, 1.00f, 1.00f, 0.0f };

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, D);

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        universe.draw(i);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

