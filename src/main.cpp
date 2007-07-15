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
#include "opengl.hpp"
#include "demo.hpp"
#include "data.hpp"
#include "conf.hpp"
#include "glob.hpp"
#include "lang.hpp"
#include "view.hpp"
#include "perf.hpp"

#define JIFFY (1000 / 60)

//-----------------------------------------------------------------------------
// Global application state.

app::conf *conf;
app::data *data;
app::prog *prog;
app::glob *glob;
app::view *view;
app::lang *lang;

//-----------------------------------------------------------------------------
// Keyboard state expression queryables.

double get_key(int i)
{
    int    n;
    Uint8 *k = SDL_GetKeyState(&n);

    if (0 <= i && i < n && k[i])
        return 1.0;
    else
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

static void video()
{
    // Look up the video mode parameters.

    int w = conf->get_i("window_w");
    int h = conf->get_i("window_h");
    int b = conf->get_i("window_b");
    int m = SDL_OPENGL;

    if (conf->get_i("window_fullscreen")) m |= SDL_FULLSCREEN;
    if (conf->get_i("window_noframe"))    m |= SDL_NOFRAME;

    // Initialize the video.

    if (SDL_SetVideoMode(w, h, b, m) == 0)
        throw std::runtime_error(SDL_GetError());

    // Initialize the OpenGL state.

    ogl::init();
}

static void init(std::string& data_file,
                 std::string& conf_file,
                 std::string& lang_file)
{
    // Initialize the global state.

    data = new app::data(data_file);
    conf = new app::conf(conf_file);
    lang = new app::lang(lang_file);

    joy  = SDL_JoystickOpen(conf->get_i("joystick"));

    // Initialize the video.

    video();

    // Initialize the demo.

    glob = new app::glob();
    view = new app::view(conf->get_i("window_w"),
                         conf->get_i("window_h"),
                         conf->get_f("view_near"),
                         conf->get_f("view_far"),
                         conf->get_f("view_zoom"));
    prog = new demo();
}

static void fini()
{
    if (view) delete view;
    if (prog) delete prog;
    if (glob) delete glob;

    if (joy) SDL_JoystickClose(joy);

    if (lang) delete lang;
    if (conf) delete conf;
    if (data) delete data;
}

//-----------------------------------------------------------------------------

static bool loop()
{
    SDL_Event e;

    // While there are available events, dispatch event handlers.

    while (SDL_PollEvent(&e))
        switch (e.type)
        {
        case SDL_QUIT:
            return false;

        case SDL_USEREVENT:
            glob->fini();
            data->load();
            conf->load();
            video();
            glob->init();
            break;

        case SDL_MOUSEMOTION:     prog->point(e.motion.x, e.motion.y);  break;
        case SDL_MOUSEBUTTONDOWN: prog->click(e.button.button,  true);  break;
        case SDL_MOUSEBUTTONUP:   prog->click(e.button.button,  false); break;
        case SDL_KEYDOWN:         prog->keybd(e.key.keysym.sym, true,
                                              e.key.keysym.unicode);    break;
        case SDL_KEYUP:           prog->keybd(e.key.keysym.sym, false,
                                              e.key.keysym.unicode);    break;
        }

    // Call the timer method for each jiffy that has passed.

    while (SDL_GetTicks() - tock >= JIFFY)
    {
        tock += JIFFY;
        prog->timer(JIFFY / 1000.0);
    }

    // Draw the scene.

    prog->draw();

    return true;
}

int main(int argc, char *argv[])
{
    std::string data_file(argc > 1 ? argv[1] : DEFAULT_DATA_FILE);
    std::string conf_file(argc > 2 ? argv[2] : DEFAULT_CONF_FILE);
    std::string lang_file(argc > 3 ? argv[3] : DEFAULT_LANG_FILE);

    try
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == 0)
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    5);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

            SDL_EnableUNICODE(1);

            init(data_file, conf_file, lang_file);
            {
                std::auto_ptr<perf> P(new perf(60));

                while (loop())
                {
                    SDL_GL_SwapBuffers();
                    P->step();
                }
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
