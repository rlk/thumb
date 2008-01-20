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

#include "matrix.hpp"
#include "opengl.hpp"
#include "tracker.hpp"
#include "solid.hpp"
#include "edit.hpp"
#include "host.hpp"
#include "conf.hpp"
#include "user.hpp"

//-----------------------------------------------------------------------------

mode::edit::edit(wrl::world &w) : mode(w)
{
    // Initialize edit mode configuration.

    key_undo             = conf->get_i("key_undo");
    key_redo             = conf->get_i("key_redo");

    key_axis_X           = conf->get_i("key_axis_X");
    key_axis_Y           = conf->get_i("key_axis_Y");
    key_axis_Z           = conf->get_i("key_axis_Z");
    key_home             = conf->get_i("key_home");

    key_position_mode    = conf->get_i("key_position_mode");
    key_rotation_mode    = conf->get_i("key_rotation_mode");

    key_make_body        = conf->get_i("key_make_body");
    key_make_nonbody     = conf->get_i("key_make_nonbody");
    key_make_joint       = conf->get_i("key_make_joint");

    key_selection_delete = conf->get_i("key_selection_delete");
    key_selection_invert = conf->get_i("key_selection_invert");
    key_selection_extend = conf->get_i("key_selection_extend");
    key_selection_clear  = conf->get_i("key_selection_clear");
    key_selection_clone  = conf->get_i("key_selection_clone");

    // Initialize edit mode state.

    drag   = false;
    move   = false;

    point_p[0] =  0;
    point_p[1] =  0;
    point_p[2] =  0;

    point_v[0] =  0;
    point_v[1] =  0;
    point_v[2] = -1;
}

//-----------------------------------------------------------------------------

bool mode::edit::point(int i, const double *p, const double *q)
{
    double M[16];

    ::user->get_point(point_p, p,
                      point_v, q);

    world.edit_pick(point_p, point_v);

    if (drag)
    {
        // If the current drag has produced a previous transform, undo it.

        if (move)
        {
            world.undo();
            move = false;
        }

        // Apply the transform of the current drag.

        if (transform.point(M, point_p, point_v))
        {
            world.do_modify(M);
            move = true;
        }

        return true;
    }
    return mode::point(i, p, q);
}

bool mode::edit::click(int i, int b, int m, bool d)
{
    if (b == SDL_BUTTON_LEFT)
    {
        drag = false;

        if (d)
        {
            // If a selected entity is clicked, a drag may be beginning.

            if (world.check_selection())
            {
                transform.click(point_p, point_v);
                drag = true;
            }
        }
        else
        {
            if (dGeomID geom = world.get_focus())
            {
                wrl::atom *focus = (wrl::atom *) dGeomGetData(geom);

                if (m & KMOD_SHIFT)
                {
                    // Shift-release resets the constraint transform.

                    double M[16];

                    if (m & KMOD_CTRL)
                        focus->get_local(M);
                    else
                        focus->get_world(M);

                    transform.set_transform(M);
                }
                else
                {
                    // A non-dragged release toggles selection.

                    if (move == false)
                        world.click_selection(focus);
                }
            }
        }

        move = false;

        return true;
    }
    return mode::click(i, b, m, d);
}

bool mode::edit::keybd(int c, int k, int m, bool d)
{
    if (d)
    {
        // Handle basic editing operations.

        if      (k == key_selection_delete) world.do_delete();
        else if (k == key_selection_invert) world.invert_selection();
        else if (k == key_selection_extend) world.extend_selection();
        else if (k == key_selection_clear)  world.clear_selection();
        else if (k == key_selection_clone)  world.clone_selection();

        else if (k == key_undo) world.undo();
        else if (k == key_redo) world.redo();

        // Handle body and joint creation and destruction.

        else if (k == key_make_body)    world.do_embody();
        else if (k == key_make_nonbody) world.do_debody();
        else if (k == key_make_joint)   world.do_enjoin();

        // Handle constraint keys.

        else if ('0' <= k && k <= '9')   transform.set_grid(int(k - '0'));
        else if (k == key_position_mode) transform.set_mode(0);
        else if (k == key_rotation_mode) transform.set_mode(1);
        else if (k == key_axis_X)        transform.set_axis(0);
        else if (k == key_axis_Y)        transform.set_axis(1);
        else if (k == key_axis_Z)        transform.set_axis(2);

        else if (k == key_home)
        {
            double M[16];

            load_idt(M);
            transform.set_transform(M);
        }

        else return false;
    }
    else return false;

    return true;
}

//-----------------------------------------------------------------------------

bool mode::edit::timer(int t)
{
    world.edit_step(0);
    return true;
}

double mode::edit::view(const double *planes)
{
    return std::max(world.view(true, planes), transform.view(planes));
}

void mode::edit::draw(const double *points)
{
    world.draw(true, points);
    transform.draw();

    if (tracker_status())
    {
        glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT |
                     GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        {
            glEnable(GL_BLEND);
            glEnable(GL_LINE_SMOOTH);

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthMask(GL_FALSE);

            glColor4f(1.0f, 1.0f, 0.0f, 0.5f);

            glLineWidth(2.0f);

            glBegin(GL_LINES);
            {
                glVertex3f(point_p[0],
                           point_p[1],
                           point_p[2]);
                glVertex3f(point_p[0] + point_v[0] * 100.0f,
                           point_p[1] + point_v[1] * 100.0f,
                           point_p[2] + point_v[2] * 100.0f);
            }
            glEnd();
        }
        glPopMatrix();
        glPopAttrib();
    }
}

//-----------------------------------------------------------------------------

