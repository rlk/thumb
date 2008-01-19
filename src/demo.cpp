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

    joy_mode   = conf->get_i("joy_mode");
    joy_axis_x = conf->get_i("joy_axis_x");
    joy_axis_y = conf->get_i("joy_axis_y");
    joy_butn_F = conf->get_i("joy_butn_F");
    joy_butn_B = conf->get_i("joy_butn_B");
    joy_butn_L = conf->get_i("joy_butn_L");
    joy_butn_R = conf->get_i("joy_butn_R");

    view_move_rate = conf->get_f("view_move_rate");
    view_turn_rate = conf->get_f("view_turn_rate");

    tracker_head_sensor = conf->get_i("tracker_head_sensor");
    tracker_hand_sensor = conf->get_i("tracker_hand_sensor");

    // Initialize the demo state.

    motion[0] = 0;
    motion[1] = 0;
    motion[2] = 0;
    rotate[0] = 0;
    rotate[1] = 0;
    rotate[2] = 0;
    button[0] = 0;
    button[1] = 0;
    button[2] = 0;
    button[3] = 0;

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

void demo::point(const double *p, const double *v)
{
    int x;
    int y;

    ::host->gui_pick(x, y, p, v);

    if (curr->point(p, v) == false)
    {
        // Handle view rotation.

        if (button[3])
        {
            if (::host->modifiers() & KMOD_SHIFT)
                ::user->turn(0, 0, -double(last_x - x) * view_turn_rate);
            else
                ::user->turn(-double(last_y - y) * view_turn_rate,
                             +double(last_x - x) * view_turn_rate, 0);
        }

        prog::point(p, v);
    }

    last_x = x;
    last_y = y;
}

void demo::click(int b, bool d)
{
    double k = 1.0;

    attr_off();

    if (joy_mode)
    {
        if (b == joy_butn_F) motion[2] = d ? +1 : 0;
        if (b == joy_butn_B) motion[2] = d ? -1 : 0;
        if (b == joy_butn_L) rotate[2] = d ? +2 : 0;
        if (b == joy_butn_R) rotate[2] = d ? -2 : 0;
    }
    else
    {
        if (::host->modifiers() & KMOD_SHIFT) k = 0.1;

        button[b] = d;

        if      (d && b == 1) ::user->home();
        else if (d && b == 2) ::user->home();
        else if (d && b == 4) universe.turn(+1.0);
        else if (d && b == 5) universe.turn(-1.0);
        else if (d && b == 0)
        {
            memcpy(init_P, curr_P, 3 * sizeof (double));
            memcpy(init_R, curr_R, 9 * sizeof (double));
        }
        else
        {
            if (curr->click(b, d) == false)
                prog::click(b, d);
        }
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

        if      (k == key_move_L) { motion[0] -= dd; attr_off(); }
        else if (k == key_move_R) { motion[0] += dd; attr_off(); }
        else if (k == key_move_F) { motion[2] -= dd; attr_off(); }
        else if (k == key_move_B) { motion[2] += dd; attr_off(); }

        else if (d)
        {
            // Handle view config keys.

            if      (k == SDLK_F5) ::user->set_type(app::user::type_mono);
            else if (k == SDLK_F6) ::user->set_type(app::user::type_anaglyph);
            else if (k == SDLK_F7) ::user->set_type(app::user::type_varrier);
            else if (k == SDLK_F8) ::user->set_type(app::user::type_scanline);
            else if (k == SDLK_F9) ::user->set_type(app::user::type_blended);

            else if (k == SDLK_HOME) ::user->home();
            else if (k == SDLK_END)  draw_sphere = !draw_sphere;

            // Handle guided view keys.

            if (::host->modifiers() & KMOD_SHIFT)
            {
                if      (k == SDLK_PAGEUP)   attr_next();
                else if (k == SDLK_PAGEDOWN) attr_prev();
                else if (k == SDLK_INSERT)   ::user->insert(universe.get_a());
                else if (k == SDLK_DELETE)   ::user->remove();
                else if (k == SDLK_SPACE)    attr_on();
            }
        }
    }

    prog::keybd(k, d, c);
}

void demo::timer(double dt)
{
    if (joy_mode)
    {
        double kp = dt * universe.rate();
        double kr = dt * view_turn_rate * 360;

        user->turn(kr * rotate[0],
                   kr * rotate[1],
                   kr * rotate[2]);
        user->move(kp * motion[0],
                   kp * motion[1],
                   kp * motion[2]);
    }
    else if (attr_mode)
    {
        double a = 0.0;

        if (user->step(dt, universe.get_p(), a))
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
        double kp, kr = 150.0 * view_turn_rate * dt;

        // Determine the rate of motion.

        if (::host->modifiers() & KMOD_CTRL)
            kp = dt * view_move_rate;
        else
            kp = dt * universe.rate();

        if (::host->modifiers() & KMOD_SHIFT)
            kp *= 10.0;

        // Handle view motion.

        user->move(motion[0] * kp,
                   motion[1] * kp,
                   motion[2] * kp);

        // Handle tracker navigation.

        if (button[0])
        {
            double dP[3];
            double dR[3];
            double dz[3];
            double dy[3];

            kp *= 0.5;

            dP[0] = curr_P[0] - init_P[0];
            dP[1] = curr_P[1] - init_P[1];
            dP[2] = curr_P[2] - init_P[2];

            dy[0] = init_R[1][0] - curr_R[1][0];
            dy[1] = init_R[1][1] - curr_R[1][1];
            dy[2] = init_R[1][2] - curr_R[1][2];

            dz[0] = init_R[2][0] - curr_R[2][0];
            dz[1] = init_R[2][1] - curr_R[2][1];
            dz[2] = init_R[2][2] - curr_R[2][2];

            dR[0] =  DOT3(dz, init_R[1]);
            dR[1] = -DOT3(dz, init_R[0]);
            dR[2] =  DOT3(dy, init_R[0]);

            user->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
            user->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
        }

        // Handle auto-attract mode.

        attr_curr += dt;

        if (attr_curr > attr_time)
            attr_on();
    }
    curr->timer(dt);
    prog::timer(dt);
}

void demo::stick(int d, const double *p)
{
    attr_off();

    if (joy_mode)
    {
        if (d == joy_axis_x && fabs(p[0]) > 0.5) rotate[1] = -p[0];
        if (d == joy_axis_y && fabs(p[0]) > 0.5) rotate[0] = +p[0];
    }
    else
    {
        if (fabs(p[0]) > 0.1)
            universe.turn(p[0]);
    }
}

void demo::track(int d, const double *p, const double *x, const double *z)
{
    double y[3];

    crossprod(y, z, x);

    if (d == tracker_head_sensor)
        ::host->set_head(p, x, y, z);

    if (d == tracker_hand_sensor)
    {
        // Point at the GUI.

        double v[3];

        v[0] = -z[0];
        v[1] = -z[1];
        v[2] = -z[2];

        point(p, v);

        // Cache the tracker position for navigation.

        curr_P[0]    = p[0];
        curr_P[1]    = p[1];
        curr_P[2]    = p[2];

        curr_R[0][0] = x[0];
        curr_R[0][1] = x[1];
        curr_R[0][2] = x[2];

        curr_R[1][0] = y[0];
        curr_R[1][1] = y[1];
        curr_R[1][2] = y[2];

        curr_R[2][0] = z[0];
        curr_R[2][1] = z[1];
        curr_R[2][2] = z[2];
    }
}

//-----------------------------------------------------------------------------

void demo::prep(const double *F, int n)
{
    universe.prep(F, n);
}

void demo::draw(const double *frag_d, const double *frag_k)
{
//  GLfloat A[4] = { 0.05f, 0.10f, 0.15f, 0.0f };
    GLfloat A[4] = { 0.45f, 0.50f, 0.55f, 0.0f };
    GLfloat D[4] = { 1.00f, 1.00f, 0.90f, 0.0f };

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
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

        user->push();
        universe.draw(frag_d, frag_k);
        user->pop();

        // Compute the view frusta.

        user->plane_frustum(planes);
        user->range(1.0, curr->view(planes));
        user->point_frustum(points);

        // Draw the scene.

        glClear(GL_DEPTH_BUFFER_BIT);

        user->push();
        user->draw();
        curr->draw(points);
        user->pop();

        if (draw_sphere)
            user->sphere();

        host->tag_draw();
    }
    glPopAttrib();
}

//-----------------------------------------------------------------------------

