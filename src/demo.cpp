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

#include "main.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "joint.hpp"
#include "demo.hpp"

//-----------------------------------------------------------------------------

demo::demo() :

    camera(), scene(camera),

    edit(scene),
    play(scene),
    info(scene)
{
    // Initialize the demo configuration.

    key_play   = conf->get_i("key_play");
    key_info   = conf->get_i("key_info");

    key_move_L = conf->get_i("key_move_L");
    key_move_R = conf->get_i("key_move_R");
    key_move_F = conf->get_i("key_move_F");
    key_move_B = conf->get_i("key_move_B");

    camera_move_rate = conf->get_f("camera_move_rate");
    camera_turn_rate = conf->get_f("camera_turn_rate");
    camera_zoom      = conf->get_f("camera_zoom");
    camera_near      = conf->get_f("camera_near");
    camera_far       = conf->get_f("camera_far");

    // Initialize the demo state.

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;
    button[1] = 0;
    button[2] = 0;
    button[3] = 0;

    curr = 0;

    goto_mode(&play);
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
    float k = camera_turn_rate;
    float p[3];
    float v[3];

    camera.pick(p, v, x, y);

    if (curr->point(p, v, x, y) == false)
    {
        // Handle camera rotation.

        if (button[3])
        {
            camera.turn_world(+float(last_x - x) * k, 0.0f, 1.0f, 0.0f);
            camera.turn_local(+float(last_y - y) * k, 1.0f, 0.0f, 0.0f);
        }
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

    if      (k == key_play)
    {
        if (d)
        {
            if (curr  ==  &play)
                goto_mode(&edit);
            else
                goto_mode(&play);
        }
    }
    else if (k == key_info)
    {
        if (d)
        {
            if (curr  ==  &info)
                goto_mode(&edit);
            else
                goto_mode(&info);
        }
    }

    // Let the current mode take it.

    else if (curr->keybd(k, d, c) == false)
    {
        int dd = d ? +1 : -1;

        // Handle camera motion keys.

        if      (k == key_move_L) motion[0] -= dd;
        else if (k == key_move_R) motion[0] += dd;
        else if (k == key_move_F) motion[2] -= dd;
        else if (k == key_move_B) motion[2] += dd;
    }

    prog::keybd(k, d, c);
}

void demo::timer(float dt)
{
    float k = camera_move_rate * dt;

    if (SDL_GetModState() & KMOD_CTRL)  k /= 4.0f;

    // Handle camera motion.

    camera.move_local(float(motion[0]) * k,
                      float(motion[1]) * k,
                      float(motion[2]) * k);

    curr->timer(dt);
    prog::timer(dt);
}

//-----------------------------------------------------------------------------

void demo::draw() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curr->draw();
    prog::draw();
}

//-----------------------------------------------------------------------------

