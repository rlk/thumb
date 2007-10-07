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
#include "host.hpp"
#include "edit.hpp"
#include "play.hpp"
#include "info.hpp"

//-----------------------------------------------------------------------------

demo::demo()
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

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;
    button[1] = 0;
    button[2] = 0;
    button[3] = 0;

    curr = 0;

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

//-----------------------------------------------------------------------------

void demo::point(const double *p, const double *v)
{
    int x;
    int y;

    ::host->gui_pick(x, y, p, v);

    if (curr->point(p, v) == false)
    {
        // Handle view rotation.

        if (button[2])
            ::view->turn(0, 0, -double(last_x - x) * view_turn_rate);

        if (button[3])
            ::view->turn(-double(last_y - y) * view_turn_rate,
                         +double(last_x - x) * view_turn_rate, 0);

        prog::point(p, v);
    }

    last_x = x;
    last_y = y;
}

void demo::click(int b, bool d)
{
/*
    if      (d && b == 4) universe.turn(+1);
    else if (d && b == 5) universe.turn(-1);
    else
*/
    {
        button[b] = d;

        if (curr->click(b, d) == false)
            prog::click(b, d);
    }
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
        else if (k == SDLK_HOME) ::view->home();
    }

    prog::keybd(k, d, c);
}

void demo::timer(double dt)
{
    double k;

    // Determine the rate of motion.
/*
    if (::host->modifiers() & KMOD_CTRL)
        k = dt * universe.rate();
    else
*/
        k = dt * view_move_rate;

    if (::host->modifiers() & KMOD_SHIFT)
        k *= 10.0;

    // Handle view motion.

    view->move(motion[0] * k,
               motion[1] * k,
               motion[2] * k);

    curr->timer(dt);
    prog::timer(dt);
}

void demo::track(int d, const double *p, const double *x, const double *z)
{
    if (d == tracker_head_sensor)
    {
        double y[3];

        crossprod(y, x, z);

        ::host->set_head(p, x, y, z);
    }

    if (d == tracker_hand_sensor)
    {
        double v[3];

        v[0] = -z[0];
        v[1] = -z[1];
        v[2] = -z[2];

        point(p, v);
    }
}

//-----------------------------------------------------------------------------

void demo::draw()
{
    GLfloat A[4] = { 0.05f, 0.10f, 0.15f, 0.0f };
    GLfloat D[4] = { 1.00f, 1.00f, 0.90f, 0.0f };

    glClearColor(0.0f, 0.1f, 0.2f, 0.0f);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, D);

    glPushAttrib(GL_ENABLE_BIT);
    {
        double planes[20];
        double points[24];

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
/*
        view->push();
        universe.draw();
        view->pop();
*/
        // Compute the view frusta.

        view->plane_frustum(planes);
        view->range(1.0, curr->view(planes));
        view->point_frustum(points);

        // Draw the scene.

        glClear(GL_DEPTH_BUFFER_BIT);

        view->push();
        view->draw();
        curr->draw(points);
        view->pop();
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

