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

#include <SDL.h>
#include <SDL_keyboard.h>

#include <ogl-opengl.hpp>
#include <ogl-uniform.hpp>

#include <app-frustum.hpp>
#include <app-event.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-host.hpp>
#include <app-glob.hpp>

#include <wrl-world.hpp>
#include <etc-math.hpp>

#include <mode-edit.hpp>
#include <mode-play.hpp>
#include <mode-info.hpp>

#include "demo.hpp"

//-----------------------------------------------------------------------------

void demo::init_uniforms()
{
    // Initialize the uniforms.

    uniform_view_matrix      = ::glob->load_uniform("view_matrix",     16);
    uniform_view_inverse     = ::glob->load_uniform("view_inverse",    16);
    uniform_view_position    = ::glob->load_uniform("view_position",    3);
    uniform_time             = ::glob->load_uniform("time",             1);
    uniform_XYZRGB           = ::glob->load_uniform("XYZRGB",           9);

    uniform_reflection_cubemap_size
        = ::glob->load_uniform("reflection_cubemap_size",  1);
    uniform_irradiance_cubemap_size
        = ::glob->load_uniform("irradiance_cubemap_size",  1);
    uniform_spherical_harmonic_order
        = ::glob->load_uniform("spherical_harmonic_order", 1);

    // Set the constant uniforms.

    uniform_reflection_cubemap_size->set(
        double(::conf->get_i("reflection_cubemap_size", 128)));
    uniform_irradiance_cubemap_size->set(
        double(::conf->get_i("irradiance_cubemap_size", 128)));
    uniform_spherical_harmonic_order->set(
        double(::conf->get_i("spherical_harmonic_order",  2)));

    static const double XYZRGB[9] = {     /* 709 */
        +3.240479, -0.969256, +0.055648,
        -1.537150, +1.875991, -0.204043,
        -0.498530, +0.041556, +1.057311
    };
    uniform_XYZRGB->set(XYZRGB);
}

void demo::free_uniforms()
{
    ::glob->free_uniform(uniform_XYZRGB);
    ::glob->free_uniform(uniform_spherical_harmonic_order);
    ::glob->free_uniform(uniform_irradiance_cubemap_size);
    ::glob->free_uniform(uniform_reflection_cubemap_size);
    ::glob->free_uniform(uniform_time);
    ::glob->free_uniform(uniform_view_position);
    ::glob->free_uniform(uniform_view_inverse);
    ::glob->free_uniform(uniform_view_matrix);
}

void demo::prep_uniforms() const
{
    const double *M = ::view->get_view_matrix();
    const double *I = ::view->get_view_inverse();

    // Set the viewing uniforms

    uniform_view_matrix  ->set(I);
    uniform_view_inverse ->set(M);
    uniform_view_position->set(M + 12);

    // The current time

    uniform_time->set(double(SDL_GetTicks() * 0.001));
}

//-----------------------------------------------------------------------------

demo::demo(const std::string& exe,
           const std::string& tag)
    : app::prog(exe, tag), world(0), edit(0), play(0), info(0), curr(0)
{
    init_uniforms();

    // Initialize the application state.

    key_info  = conf->get_i("key_info", SDL_SCANCODE_F1);
    key_edit  = conf->get_i("key_edit", SDL_SCANCODE_F2);
    key_play  = conf->get_i("key_play", SDL_SCANCODE_F3);

    world = new wrl::world();

    edit = new mode::edit(world);
    play = new mode::play(world);
    info = new mode::info(world);

    goto_mode(edit);
}

demo::~demo()
{
    free_uniforms();

    if (info)  delete info;
    if (play)  delete play;
    if (edit)  delete edit;

    if (world) delete world;
}

//-----------------------------------------------------------------------------

void demo::goto_mode(mode::mode *next)
{
    // Synthesize CLOSE and START events for the mode transition.

    app::event E;

    if (curr) { E.mk_close(); curr->process_event(&E); }
    if (next) { E.mk_start(); next->process_event(&E); }

    curr = next;
}

//-----------------------------------------------------------------------------

bool demo::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

    // Handle application mode transitions.

    if (d && !(m & KMOD_SHIFT))
    {
        if (k == key_edit && curr != edit) { goto_mode(edit); return true; }
        if (k == key_play && curr != play) { goto_mode(play); return true; }
        if (k == key_info && curr != info) { goto_mode(info); return true; }
    }
    return false;
}

bool demo::process_tick(app::event *E)
{
    // Always return IGNORED to allow other objects to process time.

    return false;
}

bool demo::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEY:   R = process_key (E); break;
    case E_TICK:  R = process_tick(E); break;
    }

    if (R) return true;

    // Allow the application base or current mode  to handle the event.

    if ((          prog::process_event(E)) ||
        (curr  &&  curr->process_event(E)))
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

ogl::range demo::prep(int frusc, const app::frustum *const *frusv)
{
    // Prep the current mode, giving the view range.

    prep_uniforms();

    ogl::range r;

    if (curr)
        r = curr->prep(frusc, frusv);
    else
        r = ogl::range();

    return r;
}

void demo::lite(int frusc, const app::frustum *const *frusv)
{
    if (curr)
        curr->lite(frusc, frusv);
}

void demo::draw(int frusi, const app::frustum *frusp, int chani)
{
    // Clear the render target.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    if (curr)
        curr->draw(frusi, frusp);
}

//-----------------------------------------------------------------------------
