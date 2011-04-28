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

#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include <SDL.h>

#include "main.hpp"
#include "util.hpp"
#include "demo.hpp"
#include "app-host.hpp"
#include "app-data.hpp"
#include "app-conf.hpp"
#include "app-glob.hpp"
#include "app-lang.hpp"
#include "app-user.hpp"
#include "app-perf.hpp"
#include "ogl-opengl.hpp"

#include "dan/danpart.hpp"

//-----------------------------------------------------------------------------
// Keyboard state expression queryables.

double get_key(int i)
{
    /*
    int    n;
    Uint8 *k = SDL_GetKeyState(&n);

    if (0 <= i && i < n && k[i])
        return 1.0;
    else
        return 0.0;
    */
    return 0.0;
}

//-----------------------------------------------------------------------------
// System time expression queryables.

static int tick = 0;
static int tock = 0;

double get_time()
{
    return (tock - tick) / 1000.0;
}

void clr_time()
{
    tick = tock;
}

//-----------------------------------------------------------------------------
// Joystick input expression queryables.

static SDL_Joystick *joy = NULL;

double get_btn(int i)
{
    if (joy)
        return SDL_JoystickGetButton(joy, i);
    else
        return 0.0;
}

double get_joy(int i)
{
    if (joy)
        return SDL_JoystickGetAxis(joy, i) / 32768.0;
    else
        return 0.0;
}

//-----------------------------------------------------------------------------
// Collision group expression queryables.

static unsigned int bits = 0;

double get_trg(unsigned int i)
{
    if (bits & (1 << i))
        return 1.0;
    else
        return 0.0;
}

void set_trg(unsigned int b) { bits |= b; }
void clr_trg()               { bits  = 0; }

//-----------------------------------------------------------------------------

static void init(std::string tag)
{
    // Initialize data access and configuration.

    data = new app::data(DEFAULT_DATA_FILE);
    conf = new app::conf(DEFAULT_CONF_FILE);

    data->init();

    // Initialize language and host configuration.

    std::string lang_conf = conf->get_s("lang_file");
    std::string host_conf = conf->get_s("host_file");

    if (lang_conf.empty()) lang_conf = DEFAULT_LANG_FILE;
    if (host_conf.empty()) host_conf = DEFAULT_HOST_FILE;

    // If the given tag is an XML file name, use it as config file.

    if (tag.size() > 4 && tag.rfind(".xml") == tag.size() - 4)
        host_conf = tag;

    lang = new app::lang(lang_conf);
    host = new app::host(host_conf, tag);

    // Initialize the OpenGL context.

    video();

    // Initialize the OpenGL state and application.

    user = new app::user();
    glob = new app::glob();
    perf = new app::perf();

#if 0
    prog = new danpart(::host->get_buffer_w(),
                       ::host->get_buffer_h());
#else
    prog = new demo(::host->get_buffer_w(),
                    ::host->get_buffer_h());
#endif

    // Initialize the controllers.

    if ((joy = SDL_JoystickOpen(conf->get_i("gamepad_device"))))
        SDL_JoystickEventState(SDL_ENABLE);
}

static void fini()
{
    if (joy) SDL_JoystickClose(joy);

    if (prog) delete prog;
    if (perf) delete perf;
    if (user) delete user;
    if (host) delete host;
    if (glob) delete glob;
    if (lang) delete lang;
    if (conf) delete conf;
    if (data) delete data;
}

//-----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == 0)
        {
            SDL_EnableUNICODE(1);

            init(std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
            {
                host->loop();
            }
            fini();

            SDL_Quit();
        }
        else throw std::runtime_error(SDL_GetError());
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

//-----------------------------------------------------------------------------
