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
#include <etc-math.hpp>

#include "uni-universe.hpp"
#include "tellurion.hpp"

//-----------------------------------------------------------------------------

void tellurion::init_uniforms()
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

void tellurion::free_uniforms()
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

void tellurion::prep_uniforms() const
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

tellurion::tellurion(const std::string& tag) : app::prog(tag), universe(0)
{
    init_uniforms();

    universe = new uni::universe(::host->get_buffer_w(),
                                 ::host->get_buffer_h());

    // Initialize attract mode.

    attr_time = conf->get_f("attract_delay");
    attr_rate = conf->get_f("attract_speed");
    attr_curr = 0.0;
    attr_sign = 1.0;
    attr_mode = false;
    attr_stop = false;

    attr_step(0.0);
}

tellurion::~tellurion()
{
    if (universe) delete universe;

    free_uniforms();
}

//-----------------------------------------------------------------------------

void tellurion::attr_on()
{
    attr_curr = 0.0;
    attr_mode = true;
    attr_stop = false;
}

void tellurion::attr_off()
{
    attr_curr = 0.0;
    attr_mode = false;
}

void tellurion::attr_step(double dt)
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

void tellurion::attr_next()
{
    // Play forward to the next key.

    attr_sign =  1.0;
    attr_curr =  0.0;
    attr_stop = true;
    attr_mode = true;
}

void tellurion::attr_prev()
{
    // Play backward to the previous key.

    attr_sign = -1.0;
    attr_curr =  0.0;
    attr_stop = true;
    attr_mode = true;
}

void tellurion::attr_ins()
{
    // Insert a new key here.

    ::user->insert(0);
    attr_step(0.0);
}

void tellurion::attr_del()
{
    // Remove the current key.

    ::user->remove();
    attr_step(0.0);
}

//-----------------------------------------------------------------------------

void tellurion::next()
{
    attr_next();
}

void tellurion::prev()
{
    attr_prev();
}

//-----------------------------------------------------------------------------

bool tellurion::process_key(app::event *E)
{
    const bool d = E->data.key.d;
    const int  k = E->data.key.k;
    const int  m = E->data.key.m;

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

bool tellurion::process_tick(app::event *E)
{
    double dt = E->data.tick.dt * 0.001;

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

bool tellurion::process_input(app::event *E)
{
    // Assume all script inputs are meaningful.  Pass them to the universe.

    if (universe)
        E->set_dst(universe->script(E->data.input.src));

    return false;
}

bool tellurion::process_event(app::event *E)
{
    bool R = false;

    // Attempt to process the given event.

    switch (E->get_type())
    {
    case E_KEY:   R = process_key  (E); break;
    case E_INPUT: R = process_input(E); break;
    case E_TICK:  R = process_tick (E); break;
    }

    if (R) return true;

    // Allow the application base to handle the event.

    if (prog::process_event(E))
    {
        ::host->set_bench_mode(0);
        ::host->set_movie_mode(0);

        attr_off();
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

ogl::range tellurion::prep(int frusc, const app::frustum *const *frusv)
{
    prep_uniforms();

    ogl::range r = ogl::range();

    if (universe)
    {
        universe->prep(frusc, frusv);

        ::user->put_move_rate(universe->get_move_rate());
    }
    return r;
}

void tellurion::lite(int frusc, const app::frustum *const *frusv)
{
}

void tellurion::draw(int frusi, const app::frustum *frusp)
{
    // Clear the render target.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    if (universe)
        universe->draw(frusi);
}

//-----------------------------------------------------------------------------
