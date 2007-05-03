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

#include <SDL.h>
#include <iostream>
#include <cstring>

#include "opengl.hpp"
#include "demo.hpp"
#include "conf.hpp"
#include "view.hpp"
#include "edit.hpp"
#include "play.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

demo::demo() : scene()
{
    edit = new mode::edit(scene);
    play = new mode::play(scene);
    info = new mode::info(scene);

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

    // Initialize the demo state.

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;
    button[1] = 0;
    button[2] = 0;
    button[3] = 0;

    curr = 0;

    goto_mode(info);
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

//-----------------------------------------------------------------------------

void demo::point(int x, int y)
{
    float k = view_turn_rate;
    float p[3];
    float v[3];

    view->pick(p, v, x, y);

    if (curr->point(p, v, x, y) == false)
    {
        // Handle view rotation.

        if (button[3])
            view->turn(float(last_y - y) * k,
                       float(last_x - x) * k, 0.0f);

        prog::point(x, y);
    }

    last_x = x;
    last_y = y;
}

void demo::click(int b, bool d)
{
    button[b] = d;

    if (curr->click(b, d) == false)
        prog::click(b, d);
}

void demo::keybd(int k, bool d, int c)
{
    // Handle mode transitions.

    if      (d && k == key_edit && curr != edit) goto_mode(edit);
    else if (d && k == key_play && curr != play) goto_mode(play);
    else if (d && k == key_info && curr != info) goto_mode(info);

    // Let the current mode take it.

    else if (curr->keybd(k, d, c) == false)
    {
        int dd = d ? +1 : -1;

        // Handle view motion keys.

        if      (k == key_move_L) motion[0] -= dd;
        else if (k == key_move_R) motion[0] += dd;
        else if (k == key_move_F) motion[2] -= dd;
        else if (k == key_move_B) motion[2] += dd;
    }

    prog::keybd(k, d, c);
}

void demo::timer(float dt)
{
    float k = view_move_rate * dt;

    if (SDL_GetModState() & KMOD_CTRL)  k /= 4.0f;

    // Handle view motion.

    view->move(float(motion[0]) * k,
               float(motion[1]) * k,
               float(motion[2]) * k);

    curr->timer(dt);
    prog::timer(dt);
}

//-----------------------------------------------------------------------------

void demo::draw()
{
    float A[4] = { 0.2f, 0.2f, 0.2f, 0.0f };
//  float A[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearColor(0.1f, 0.2f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);

    glPushAttrib(GL_ENABLE_BIT);
    {
        // Draw the scene.

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);

        curr->draw();
        prog::draw();
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

