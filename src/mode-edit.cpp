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

#include <cassert>

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "matrix.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-event.hpp"
#include "app-frustum.hpp"
#include "wrl-world.hpp"
#include "wrl-solid.hpp"
#include "wrl-constraint.hpp"
#include "mode-edit.hpp"

//-----------------------------------------------------------------------------

mode::edit::edit(wrl::world *w) : mode(w)
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

    xform = new wrl::constraint;
}

mode::edit::~edit()
{
    delete xform;
}

//-----------------------------------------------------------------------------

bool mode::edit::process_point(app::event *E)
{
    assert(E);

    if (E->data.point.i == 0)
    {
        assert(world);
        assert(xform);

        // Transform the point event into world space.

        ::user->get_point(point_p, E->data.point.p,
                          point_v, E->data.point.q);

        world->edit_pick(point_p, point_v);

        // If there is a drag in progress...

        if (drag)
        {
            double M[16];

            // If the current drag has produced a previous transform, undo it.
            
            if (move)
            {
                world->undo();
                move = false;
            }

            // Apply the transform of the current drag.

            if (xform->point(M, point_p, point_v))
            {
                world->do_modify(M);
                move = true;
            }

            return true;
        }
    }
    return false;
}

bool mode::edit::process_click(app::event *E)
{
    assert(E);
    assert(world);
    assert(xform);

    const int  b = E->data.click.b;
    const int  m = E->data.click.m;
    const bool d = E->data.click.d;

    if (b == SDL_BUTTON_LEFT)
    {
        drag = false;

        if (d)
        {
            // If a selected entity is clicked, a drag may be beginning.

            if (world->check_selection())
            {
                xform->click(point_p, point_v);
                drag = true;
            }
        }
        else
        {
            if (dGeomID geom = world->get_focus())
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

                    xform->set_transform(M);
                }
                else
                {
                    // A non-dragged release toggles selection.

                    if (move == false)
                        world->click_selection(focus);
                }
            }
        }

        move = false;

        return true;
    }
    return false;
}

bool mode::edit::process_keybd(app::event *E)
{
    assert(E);
    assert(world);
    assert(xform);

    const bool d = E->data.keybd.d;
    const int  k = E->data.keybd.k;
    const int  m = E->data.keybd.m;

    if (d && !(m & KMOD_CTRL))
    {
        // Handle basic editing operations.

        if      (k == key_selection_delete)
        {
            world->do_delete();
            return true;
        }
        else if (k == key_selection_invert)
        {
            world->invert_selection();
            return true;
        }
        else if (k == key_selection_extend)
        {
            world->extend_selection();
            return true;
        }
        else if (k == key_selection_clear)
        {
            world->clear_selection();
            return true;
        }
        else if (k == key_selection_clone)
        {
            world->clone_selection();
            return true;
        }

        // Handle undo and redo.

        else if (k == key_undo)
        {
            world->undo();
            return true;
        }
        else if (k == key_redo)
        {
            world->redo();
            return true;
        }

        // Handle body and joint creation and destruction.

        else if (k == key_make_body)
        {
            world->do_embody();
            return true;
        }
        else if (k == key_make_nonbody)
        {
            world->do_debody();
            return true;
        }
        else if (k == key_make_joint)
        {
            world->do_enjoin();
            return true;
        }

        // Handle constraint keys.

        else if ('0' <= k && k <= '9')
        {
            xform->set_grid(int(k - '0'));
            return true;
        }
        else if (k == key_position_mode)
        {
            xform->set_mode(0);
            return true;
        }
        else if (k == key_rotation_mode)
        {
            xform->set_mode(1);
            return true;
        }
        else if (k == key_axis_X)
        {
            xform->set_axis(0);
            return true;
        }
        else if (k == key_axis_Y)
        {
            xform->set_axis(1);
            return true;
        }
        else if (k == key_axis_Z)
        {
            xform->set_axis(2);
            return true;
        }
        else if (k == key_home)
        {
            if (m & KMOD_SHIFT)
                ::user->home();
            else
            {
                double M[16];
                load_idt(M);
                xform->set_transform(M);
            }
            return true;
        }

        // Handle time (lighting) keys.

        else if (k == SDLK_LEFT)
        {
            if (m & KMOD_SHIFT) ::user->pass(-3600.0);
            else                ::user->pass( -600.0);
            return true;
        }
        else if (k == SDLK_RIGHT)
        {
            if (m & KMOD_SHIFT) ::user->pass(+3600.0);
            else                ::user->pass( +600.0);
            return true;
        }
    }
    return false;
}

bool mode::edit::process_timer(app::event *E)
{
    assert(world);

    world->edit_step(0);
    return false;
}

//-----------------------------------------------------------------------------

ogl::range mode::edit::prep(int frusc, const app::frustum **frusv)
{
    assert(world);
    assert(xform);

    // Prep the world and the constraint.  Combine the ranges.
    
    ogl::range r;

    r.merge(world->prep_fill(frusc, frusv));
    r.merge(world->prep_line(frusc, frusv));
    r.merge(xform->prep     (frusc, frusv));

//  world->prep_lite(frusc, frusv, r);

    return r;
}

void mode::edit::draw(int frusi, const app::frustum *frusp)
{
    assert(world);
    assert(xform);

    // Draw the world and the constraint.

     frusp->draw();
    ::user->draw();

    world->draw_fill(frusi, frusp);
    world->draw_line(frusi, frusp);
    xform->draw     (frusi, frusp);
/*
    ogl::line_state_init();
    glBegin(GL_LINES);
    {
        double L = 100.0;
        glVertex3d(point_p[0], point_p[1], point_p[2]);
        glVertex3d(point_p[0] + point_v[0] * L,
                   point_p[1] + point_v[1] * L,
                   point_p[2] + point_v[2] * L);
    }
    glEnd();
    ogl::line_state_fini();
*/
}

//-----------------------------------------------------------------------------

bool mode::edit::process_event(app::event *E)
{
    assert(E);

    bool R = false;

    switch (E->get_type())
    {
    case E_POINT: R |= process_point(E); break;
    case E_CLICK: R |= process_click(E); break;
    case E_KEYBD: R |= process_keybd(E); break;
    case E_TIMER: R |= process_timer(E); break;
    }

    return R || mode::process_event(E);
}

//-----------------------------------------------------------------------------

