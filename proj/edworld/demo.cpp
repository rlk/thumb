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

#include <app-event.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-glob.hpp>

#include <wrl-world.hpp>
#include <etc-math.hpp>

#include <dev-mouse.hpp>
#include <dev-hybrid.hpp>
#include <dev-gamepad.hpp>
#include <dev-tracker.hpp>

#include <mode-edit.hpp>
#include <mode-play.hpp>
#include <mode-info.hpp>

#include "demo.hpp"

//-----------------------------------------------------------------------------

void demo::init_uniforms()
{
    // Initialize the uniforms.

    uniform_light_position   = ::glob->load_uniform("light_position",   3);
    uniform_light_theta_cos  = ::glob->load_uniform("light_theta_cos",  1);
    uniform_light_theta      = ::glob->load_uniform("light_theta",      1);

    uniform_view_matrix      = ::glob->load_uniform("view_matrix",     16);
    uniform_view_inverse     = ::glob->load_uniform("view_inverse",    16);
    uniform_view_position    = ::glob->load_uniform("view_position",    3);
    uniform_time             = ::glob->load_uniform("time",             1);
    uniform_color_max        = ::glob->load_uniform("color_max",        4);

    uniform_XYZRGB           = ::glob->load_uniform("XYZRGB",           9);

    uniform_perez[0]         = ::glob->load_uniform("perez[0]",         3);
    uniform_perez[1]         = ::glob->load_uniform("perez[1]",         3);
    uniform_perez[2]         = ::glob->load_uniform("perez[2]",         3);
    uniform_perez[3]         = ::glob->load_uniform("perez[3]",         3);
    uniform_perez[4]         = ::glob->load_uniform("perez[4]",         3);

    uniform_zenith_luminance = ::glob->load_uniform("zenith_luminance", 3);

    uniform_reflection_cubemap_size
        = ::glob->load_uniform("reflection_cubemap_size",  1);
    uniform_irradiance_cubemap_size
        = ::glob->load_uniform("irradiance_cubemap_size",  1);
    uniform_spherical_harmonic_order
        = ::glob->load_uniform("spherical_harmonic_order", 1);

    // Set the constant uniforms.

    uniform_color_max->set(0.0, 0.0, 0.0, 0.0);

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
    ::glob->free_uniform(uniform_zenith_luminance);
    ::glob->free_uniform(uniform_perez[4]);
    ::glob->free_uniform(uniform_perez[3]);
    ::glob->free_uniform(uniform_perez[2]);
    ::glob->free_uniform(uniform_perez[1]);
    ::glob->free_uniform(uniform_perez[0]);
    ::glob->free_uniform(uniform_XYZRGB);
    ::glob->free_uniform(uniform_spherical_harmonic_order);
    ::glob->free_uniform(uniform_irradiance_cubemap_size);
    ::glob->free_uniform(uniform_reflection_cubemap_size);
    ::glob->free_uniform(uniform_color_max);
    ::glob->free_uniform(uniform_time);
    ::glob->free_uniform(uniform_view_position);
    ::glob->free_uniform(uniform_view_inverse);
    ::glob->free_uniform(uniform_view_matrix);
    ::glob->free_uniform(uniform_light_theta);
    ::glob->free_uniform(uniform_light_theta_cos);
    ::glob->free_uniform(uniform_light_position);
}

void demo::prep_uniforms() const
{
    const double T = 3.0;

    const double *M = ::user->get_M();
    const double *I = ::user->get_I();
    const double *L = ::user->get_L();

    // Set the viewing uniforms

    uniform_view_matrix  ->set(I);
    uniform_view_inverse ->set(M);
    uniform_view_position->set(M + 12);

    // The lighting uniforms

    double y = L[1] / sqrt(DOT3(L, L)), t = acos(y);

    uniform_light_position ->set(L);
    uniform_light_theta    ->set(t);
    uniform_light_theta_cos->set(y);

    uniform_color_max->set(0.0, 0.0, 0.0, 0.0);

    // The current time

    uniform_time->set(double(SDL_GetTicks() * 0.001));

    // Perez model luminance distribution coefficients

    static const double perez_Y[5][2] = {
        {  0.1787, -1.4630 },
        { -0.3554,  0.4275 },
        { -0.0227,  5.3251 },
        {  0.1206, -2.5771 },
        { -0.0670,  0.2703 }
    };
    static const double perez_x[5][2] = {
        { -0.0193, -0.2592 },
        { -0.0665,  0.0008 },
        { -0.0004,  0.2125 },
        { -0.0641, -0.8989 },
        { -0.0033,  0.0452 }
    };
    static const double perez_y[5][2] = {
        { -0.0167, -0.2608 },
        { -0.0950,  0.0092 },
        { -0.0079,  0.2102 },
        { -0.0441, -1.6537 },
        { -0.0109,  0.0529 }
    };

    for (int i = 0; i < 5; ++i)
        uniform_perez[i]->set(perez_Y[i][0] * T + perez_Y[i][1],
                              perez_x[i][0] * T + perez_x[i][1],
                              perez_y[i][0] * T + perez_y[i][1]);

    // Zenith luminance

    const double T2 = T  * T;
    const double t2 = t  * t;
    const double t3 = t2 * t;

    double Z[3], chi = (4.0 / 9.0 - T / 120.0) * (M_PI - 2.0 * t);

    Z[0] = ((4.0453 * T - 4.9710) * tan(chi) - 0.2155 * T + 2.4192);

    Z[1] = (( 0.00166 * t3 - 0.00375 * t2 + 0.00209 * t + 0.00000) * T2 +
            (-0.02903 * t3 + 0.06377 * t2 - 0.03202 * t + 0.00394) * T  +
            ( 0.11693 * t3 - 0.21196 * t2 + 0.06052 * t + 0.25886));

    Z[2] = (( 0.00275 * t3 - 0.00610 * t2 + 0.00317 * t + 0.00000) * T2 +
            (-0.04214 * t3 + 0.08970 * t2 - 0.04153 * t + 0.00516) * T  +
            ( 0.15346 * t3 - 0.26756 * t2 + 0.06670 * t + 0.26688));

    uniform_zenith_luminance->set(Z);
}

//-----------------------------------------------------------------------------

demo::demo(const std::string& tag)
    : app::prog(tag), world(0), edit(0), play(0), info(0), curr(0), input(0)
{
    std::string input_mode = conf->get_s("input_mode");

    init_uniforms();

    // Initialize the input handler.

    if      (input_mode == "hybrid")  input = new dev::hybrid();
    else if (input_mode == "gamepad") input = new dev::gamepad();
    else if (input_mode == "tracker") input = new dev::tracker();
//  else if (input_mode == "wiimote") input = new dev::wiimote();
    else                              input = new dev::mouse  ();

    // Initialize attract mode.

    attr_time = conf->get_f("attract_delay");
    attr_rate = conf->get_f("attract_speed");
    attr_curr = 0.0;
    attr_sign = 1.0;
    attr_mode = false;
    attr_stop = false;

    // Initialize the application state.

    key_edit  = conf->get_i("key_edit");
    key_play  = conf->get_i("key_play");
    key_info  = conf->get_i("key_info");

    world = new wrl::world();

    edit = new mode::edit(world);
    play = new mode::play(world);
    info = new mode::info(world);

    goto_mode(edit);

    attr_step(0.0);
}

demo::~demo()
{
    free_uniforms();

    if (info) delete info;
    if (play) delete play;
    if (edit) delete edit;

    if (world)    delete world;
    if (input)    delete input;
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

void demo::attr_on()
{
    attr_curr = 0.0;
    attr_mode = true;
    attr_stop = false;
}

void demo::attr_off()
{
    attr_curr = 0.0;
    attr_mode = false;
}

void demo::attr_step(double dt)
{
    // Move the camera forward.

    if (attr_mode)
    {
        int opts = 0;

        if (user->dostep(attr_rate * attr_sign * dt, opts))
        {
            if (attr_stop)
                attr_off();
        }
    }
}

void demo::attr_next()
{
    // Play forward to the next key.

    attr_sign =  1.0;
    attr_curr =  0.0;
    attr_stop = true;
    attr_mode = true;
}

void demo::attr_prev()
{
    // Play backward to the previous key.

    attr_sign = -1.0;
    attr_curr =  0.0;
    attr_stop = true;
    attr_mode = true;
}

void demo::attr_ins()
{
    // Insert a new key here.

    ::user->insert(0);
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

bool demo::process_keybd(app::event *E)
{
    const bool d = E->data.keybd.d;
    const int  k = E->data.keybd.k;
    const int  m = E->data.keybd.m;

    // Handle application mode transitions.

    if (d && !(m & KMOD_SHIFT))
    {
        if (k == key_edit && curr != edit) { goto_mode(edit); return true; }
        if (k == key_play && curr != play) { goto_mode(play); return true; }
        if (k == key_info && curr != info) { goto_mode(info); return true; }
    }

    // Handle attract mode controls.

    if (d &&  (m & KMOD_SHIFT))
    {
        if      (k == SDLK_PAGEUP)   { attr_next(); return true; }
        else if (k == SDLK_PAGEDOWN) { attr_prev(); return true; }
        else if (k == SDLK_END)      { attr_ins();  return true; }
        else if (k == SDLK_HOME)     { attr_del();  return true; }

        else if (k == SDLK_SPACE)
        {
            if (m & KMOD_CTRL)
            {
                ::host->set_bench_mode(1);
                ::host->set_movie_mode(2);
            }
            attr_on();
            return true;
        }
    }

    return false;
}

bool demo::process_timer(app::event *E)
{
    double dt = E->data.timer.dt * 0.001;

    // 

    ::user->auto_step(dt);

    // Step attract mode, if enabled.

    if (attr_mode)
        attr_step(dt);

    // Otherwise, if the attract delay has expired, enable attract mode.

    else
    {
        attr_curr += dt;

        if (attr_curr > attr_time)
            attr_on();
    }

    // Always return IGNORED to allow other objects to process time.

    return false;
}

bool demo::process_input(app::event *E)
{
    return false;
}

bool demo::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEYBD: R = process_keybd(E); break;
    case E_INPUT: R = process_input(E); break;
    case E_TIMER: R = process_timer(E); break;
    }

    if (R) return true;

    // Allow the application mode, the device, or the base to handle the event.

    if ((          prog::process_event(E)) ||
        (curr  &&  curr->process_event(E)) ||
        (input && input->process_event(E)))

        // If the event was handled, disable the attract mode.
    {
        ::host->set_bench_mode(0);
        ::host->set_movie_mode(0);

        attr_off();
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

void demo::draw(int frusi, const app::frustum *frusp)
{
    // Clear the render target.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    if (curr)
        curr->draw(frusi, frusp);
}

//-----------------------------------------------------------------------------
