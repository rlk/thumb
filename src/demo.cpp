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
    button[0] = 0;
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
        {
            if (::host->modifiers() & KMOD_SHIFT)
                ::view->rot_plane(+double(last_y - y) * view_turn_rate,
                                  -double(last_x - x) * view_turn_rate);
            else
                ::view->turn(-double(last_y - y) * view_turn_rate,
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

    if (::host->modifiers() & KMOD_SHIFT) k = 0.1;

    button[b] = d;

    if      (d && b == 1) ::view->home();
    else if (d && b == 4) ::view->mov_plane(+k);
    else if (d && b == 5) ::view->mov_plane(-k);
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

        // Handle view config keys.

        else if (k == SDLK_F5) ::view->set_type(app::view::type_mono);
        else if (k == SDLK_F6) ::view->set_type(app::view::type_anaglyph);
        else if (k == SDLK_F7) ::view->set_type(app::view::type_varrier);

        else if (d && k == SDLK_INSERT)
        {
            int t = conf->get_i("twiddle") + 1;
            conf->set_i("twiddle", t);
        }
        else if (d && k == SDLK_DELETE)
        {
            int t = conf->get_i("twiddle") - 1;
            conf->set_i("twiddle", t);
        }

        // Handle Varrier calibration keys.

        if (d && ::host->modifiers() & KMOD_CTRL)
        {
            if (k == SDLK_TAB)
            {
                if (::view->get_mode() == app::view::mode_norm)
                    ::view->set_mode(app::view::mode_test);
                else
                    ::view->set_mode(app::view::mode_norm);
            }
            else if (k == SDLK_PAGEUP)    ::host->set_varrier_index(+1);
            else if (k == SDLK_PAGEDOWN)  ::host->set_varrier_index(-1);

            if (::host->modifiers() & KMOD_SHIFT)
            {
                if      (k == SDLK_LEFT)  ::host->set_varrier_angle(-0.01);
                else if (k == SDLK_RIGHT) ::host->set_varrier_angle(+0.01);
                else if (k == SDLK_DOWN)  ::host->set_varrier_pitch(-0.01);
                else if (k == SDLK_UP)    ::host->set_varrier_pitch(+0.01);
            }
            else
            {
                if      (k == SDLK_LEFT)  ::host->set_varrier_shift(-0.00005);
                else if (k == SDLK_RIGHT) ::host->set_varrier_shift(+0.00005);
                else if (k == SDLK_DOWN)  ::host->set_varrier_thick(-0.0001);
                else if (k == SDLK_UP)    ::host->set_varrier_thick(+0.0001);
            }
        }
    }

    prog::keybd(k, d, c);
}

void demo::timer(double dt)
{
    double kp;
    double kr = 150.0 * view_turn_rate * dt;

    // Determine the rate of motion.

    if (::host->modifiers() & KMOD_CTRL)
        kp = dt * view_move_rate;
    else
        kp = dt * universe.rate();

    if (::host->modifiers() & KMOD_SHIFT)
        kp *= 10.0;

    // Handle view motion.

    view->move(motion[0] * kp,
               motion[1] * kp,
               motion[2] * kp);

    // Handle tracker navigation.

    if (button[0])
    {
        double dP[3];
        double dR[3];
        double dz[3];
        double dy[3];

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

        view->turn(dR[0] * kr, dR[1] * kr, dR[2] * kr, curr_R);
        view->move(dP[0] * kp, dP[1] * kp, dP[2] * kp);
    }

    curr->timer(dt);
    prog::timer(dt);
}

void demo::stick(int d, const double *p)
{
    if (fabs(p[0]) > 0.1)
        universe.turn(p[0]);
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

        view->push();
        universe.draw(frag_d, frag_k);
        view->pop();

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

