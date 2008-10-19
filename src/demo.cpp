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

#include <iostream>
#include <cstring>

#include "ogl-opengl.hpp"
#include "demo.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-host.hpp"
#include "dev-mouse.hpp"
#include "dev-gamepad.hpp"
#include "dev-tracker.hpp"
#include "dev-wiimote.hpp"
#include "mode-edit.hpp"
#include "mode-play.hpp"
#include "mode-info.hpp"

//-----------------------------------------------------------------------------

demo::demo() : edit(0), play(0), info(0), curr(0), input(0)
{
    std::string input_mode = conf->get_s("input_mode");

    // Initialize the input handler.

    if      (input_mode == "gamepad") input = new dev::gamepad(universe);
    else if (input_mode == "tracker") input = new dev::tracker(universe);
    else if (input_mode == "wiimote") input = new dev::wiimote(universe);
    else                              input = new dev::mouse  (universe);

    // Initialize attract mode.

    attr_time = conf->get_f("attract_delay");
    attr_rate = conf->get_f("attract_speed");
    attr_curr = 0;
    attr_mode = false;

    // Initialize the application state.

    key_edit  = conf->get_i("key_edit");
    key_play  = conf->get_i("key_play");
    key_info  = conf->get_i("key_info");

//  edit = new mode::edit(world);
//  play = new mode::play(world);
//  info = new mode::info(world);

//  goto_mode(play);

//  if (::conf->get_i("movie")) attr_on();

    attr_step(0.0);
}

demo::~demo()
{
    if (input) delete input;

    if (info) delete info;
    if (play) delete play;
    if (edit) delete edit;
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
    attr_curr = 0.0;
}

void demo::attr_off()
{
    attr_mode = false;
    attr_curr = 0.0;
}

void demo::attr_step(double dt)
{
    // Move the camera forward and update the universe.

    double time = 0.0;
    int    opts =   0;

    if (user->dostep(dt * attr_rate, universe.move_rate(), time, opts))
    {
        universe.set_time(time);
        set_options(opts);
    }
}

void demo::attr_next()
{
    // Teleport to the next key.

    attr_off();
    ::user->gonext();
    attr_step(0.0);
}

void demo::attr_prev()
{
    // Teleport to the previous key.

    attr_off();
    ::user->goprev();
    attr_step(0.0);
}

void demo::attr_ins()
{
    // Insert a new key here and update the universe.

    ::user->insert(universe.get_time(), get_options());
    attr_step(0.0);
}

void demo::attr_del()
{
    // Remove the current key.

    ::user->remove();
    attr_step(0.0);
}

//-----------------------------------------------------------------------------

void demo::next()
{
    attr_next();
}

void demo::prev()
{
    attr_prev();
}

//-----------------------------------------------------------------------------

void demo::point(int i, const double *p, const double *q)
{
    if (input && input->point(i, p, q))
        attr_off();
}

void demo::click(int i, int b, int m, bool d)
{
    if (input && input->click(i, b, m, d))
        attr_off();

    if (curr)
    {
        if (curr->click(i, b, m, d) == false)
            prog::click(i, b, m, d);
    }
}

void demo::keybd(int c, int k, int m, bool d)
{
    if (input && input->keybd(c, k, m, d))
        attr_off();
    else
    {
        prog::keybd(c, k, m, d);

        // Handle mode transitions.

        if      (d && k == key_edit && curr != edit) goto_mode(edit);
        else if (d && k == key_play && curr != play) goto_mode(play);
        else if (d && k == key_info && curr != info) goto_mode(info);

        // Let the current mode take it.

        if (curr == 0 || curr->keybd(c, k, m, d) == false)
        {
            // Handle guided view keys.

            if (d && (m & KMOD_SHIFT))
            {
                if      (k == SDLK_PAGEUP)   attr_next();
                else if (k == SDLK_PAGEDOWN) attr_prev();
                else if (k == SDLK_END)      attr_ins();
                else if (k == SDLK_HOME)     attr_del();
                else if (k == SDLK_SPACE)    attr_on();
            }
        }
    }
}

void demo::timer(int t)
{
    double dt = t / 1000.0;

    if (attr_mode)
        attr_step(dt);
    else
    {
        if (input && input->timer(t))
            attr_off();
        else
        {
            // If the attract delay has expired, enable attract mode.

            attr_curr += dt;

            if (attr_curr > attr_time)
                attr_on();
        }
    }

    if (curr)
        curr->timer(t);
    else
        prog::timer(t);
}

void demo::value(int d, int a, double v)
{
    if (input && input->value(d, a, v))
        attr_off();
}

//-----------------------------------------------------------------------------

void demo::prep(app::frustum_v& frusta)
{
    universe.prep(frusta);
}

void demo::draw(int i)
{
/*
    GLfloat A[4] = { 0.25f, 0.25f, 0.25f, 0.0f };
*/
    if (::prog->option(2))
    {
        GLfloat A[4] = { 0.10f, 0.10f, 0.10f, 0.0f };
        GLfloat D[4] = { 0.90f, 0.90f, 0.90f, 0.0f };

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, D);
    }
    else
    {
        GLfloat A[4] = { 0.45f, 0.45f, 0.45f, 0.0f };
        GLfloat D[4] = { 1.00f, 1.00f, 0.90f, 0.0f };

        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, A);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, D);
    }

    glPushAttrib(GL_ENABLE_BIT);
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        universe.draw(i);
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

