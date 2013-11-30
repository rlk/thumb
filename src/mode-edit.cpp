//  Copyright (C) 2005-2011 Robert Kooima
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

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include <etc-vector.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <wrl-world.hpp>
#include <wrl-solid.hpp>
#include <wrl-constraint.hpp>
#include <mode-edit.hpp>

//-----------------------------------------------------------------------------

mode::edit::edit(wrl::world *w) : mode(w)
{
    // Initialize edit mode configuration.

    key_undo             = conf->get_i("key_undo",             SDL_SCANCODE_U);
    key_redo             = conf->get_i("key_redo",             SDL_SCANCODE_V);

    key_axis_X           = conf->get_i("key_axis_X",           SDL_SCANCODE_X);
    key_axis_Y           = conf->get_i("key_axis_Y",           SDL_SCANCODE_Y);
    key_axis_Z           = conf->get_i("key_axis_Z",           SDL_SCANCODE_Z);
    key_home             = conf->get_i("key_home",             SDL_SCANCODE_RETURN);

    key_position_mode    = conf->get_i("key_position_mode",    SDL_SCANCODE_T);
    key_rotation_mode    = conf->get_i("key_rotation_mode",    SDL_SCANCODE_R);

    key_make_body        = conf->get_i("key_make_body",        SDL_SCANCODE_B);
    key_make_nonbody     = conf->get_i("key_make_nonbody",     SDL_SCANCODE_N);
    key_make_joint       = conf->get_i("key_make_joint",       SDL_SCANCODE_J);

    key_selection_delete = conf->get_i("key_selection_delete", SDL_SCANCODE_BACKSPACE);
    key_selection_invert = conf->get_i("key_selection_invert", SDL_SCANCODE_I);
    key_selection_extend = conf->get_i("key_selection_extend", SDL_SCANCODE_G);
    key_selection_clear  = conf->get_i("key_selection_clear",  SDL_SCANCODE_SPACE);
    key_selection_clone  = conf->get_i("key_selection_clone",  SDL_SCANCODE_C);

    // Initialize edit mode state.

    drag   = false;
    move   = false;

    point_p = vec3(0,  0,  0);
    point_v = vec3(0,  0, -1);

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

        point_p = ::view->get_point_pos(vec3(E->data.point.p[0],
                                             E->data.point.p[1],
                                             E->data.point.p[2]));
        point_v = ::view->get_point_vec(quat(E->data.point.q[0],
                                             E->data.point.q[1],
                                             E->data.point.q[2],
                                             E->data.point.q[3]));

        world->edit_pick(point_p, point_v);

        // If there is a drag in progress...

        if (drag)
        {
            // If the current drag has produced a previous transform, undo it.

            if (move)
            {
                world->undo();
                move = false;
            }

            // Apply the transform of the current drag.

            mat4 M;

            if (xform->point(point_p, point_v, M))
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

                    mat4 M;

                    if (m & KMOD_CTRL)
                        M = focus->get_local();
                    else
                        M = focus->get_world();

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

bool mode::edit::process_key(app::event *E)
{
    assert(E);
    assert(world);
    assert(xform);

    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

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

        else if (k == SDL_SCANCODE_0)
        {
            xform->set_grid(0);
            return true;
        }
        else if (k == SDL_SCANCODE_1)
        {
            xform->set_grid(1);
            return true;
        }
        else if (k == SDL_SCANCODE_2)
        {
            xform->set_grid(2);
            return true;
        }
        else if (k == SDL_SCANCODE_3)
        {
            xform->set_grid(3);
            return true;
        }
        else if (k == SDL_SCANCODE_4)
        {
            xform->set_grid(4);
            return true;
        }
        else if (k == SDL_SCANCODE_5)
        {
            xform->set_grid(5);
            return true;
        }
        else if (k == SDL_SCANCODE_6)
        {
            xform->set_grid(6);
            return true;
        }
        else if (k == SDL_SCANCODE_7)
        {
            xform->set_grid(7);
            return true;
        }
        else if (k == SDL_SCANCODE_8)
        {
            xform->set_grid(8);
            return true;
        }
        else if (k == SDL_SCANCODE_9)
        {
            xform->set_grid(9);
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
                ::view->go_home();
            else
                xform->set_transform(mat4());
            return true;
        }
    }
    return false;
}

bool mode::edit::process_tick(app::event *E)
{
    assert(world);

    world->edit_step(0);
    return false;
}

//-----------------------------------------------------------------------------

ogl::range mode::edit::prep(int frusc, const app::frustum *const *frusv)
{
    assert(world);
    assert(xform);

    // Prep the world and the constraint.  Combine the ranges.

    ogl::range r;

    r.merge(world->prep_fill(frusc, frusv));
    r.merge(world->prep_line(frusc, frusv));
    r.merge(xform->prep     (frusc, frusv));

    return r;
}

void mode::edit::draw(int frusi, const app::frustum *frusp)
{
    assert(world);
    assert(xform);

    // Draw the world and the constraint.

     frusp->load_transform();
    ::view->load_transform();

    world->draw_fill(frusi, frusp);
    world->draw_line(frusi, frusp);
    xform->draw     (frusi);
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
    case E_KEY:   R |= process_key(E); break;
    case E_TICK:  R |= process_tick(E); break;
    }

    return R || mode::process_event(E);
}

//-----------------------------------------------------------------------------

