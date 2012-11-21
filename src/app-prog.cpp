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

#include <SDL.h>
#include <png.h>

#include <stdexcept>

#include <ogl-opengl.hpp>
#include <app-event.hpp>

#include <app-prog.hpp>
#include <app-conf.hpp>
#include <app-data.hpp>
#include <app-glob.hpp>
#include <app-user.hpp>
#include <app-lang.hpp>
#include <app-host.hpp>
#include <app-perf.hpp>

#include <dev-mouse.hpp>
#include <dev-hybrid.hpp>
#include <dev-trackd.hpp>
#include <dev-gamepad.hpp>

//-----------------------------------------------------------------------------
// Global application state

app::conf *conf = 0;
app::data *data = 0;
app::glob *glob = 0;
app::user *user = 0;
app::lang *lang = 0;
app::host *host = 0;
app::perf *perf = 0;

//-----------------------------------------------------------------------------

#define XTR(S) #S
#define STR(S) XTR(S)

static const char *version = "Thumb Version " STR(VERSION);

//-----------------------------------------------------------------------------

static void position(int x, int y)
{
    // SDL looks to the environment for window position.

    char buf[256];

    sprintf(buf, "SDL_VIDEO_WINDOW_POS=%d,%d", x, y);
    SDL_putenv(buf);
}

static void video()
{
    // Look up the video mode parameters.

    int m = ::host->get_window_m() | SDL_OPENGL;
    int x = ::host->get_window_x();
    int y = ::host->get_window_y();
    int w = ::host->get_window_w();
    int h = ::host->get_window_h();

    // Unframed windows have no cursor and may be positioned.

    if (m & SDL_NOFRAME)
    {
//      SDL_ShowCursor(SDL_DISABLE);

        if ((m & SDL_FULLSCREEN) == 0)
            position(x, y);
    }

    // Look up the GL context parameters.

    int mults = conf->get_i("multisample_samples");
    int multb = conf->get_i("multisample_buffers");
    int color =  8;
    int depth = 24;

    for (;;)
    {
        // Configure the GL context.

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,           color);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,         color);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,          color);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         depth);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, multb);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mults);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,           1);

        // Attempt to initialize the video mode.

        if (SDL_SetVideoMode(w, h, 0, m))
            break;
        else
        {
            // If failed, try reducing the requirements.

            if      (mults >  0) mults /=  2;
            else if (multb >  0) multb  =  0;
            else if (depth > 16) depth  = 16;
            else if (color >  5) color  =  5;

            // After all reductions, fail.

            else throw std::runtime_error(SDL_GetError());
        }
    }

    // Initialize the OpenGL state.

    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multb);
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &mults);

    glewInit();

    ogl::init(multb > 0 && mults > 0);

    SDL_WM_SetCaption(version, version);
}

//-----------------------------------------------------------------------------

app::prog::prog(const std::string& exe,
                const std::string& tag) : input(0)
{
    // Start Winsock

#ifdef _WIN32
    WSADATA wsadata;
    WSAStartup(0x202, &wsadata);
#endif

    // Start SDL

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
        throw std::runtime_error(SDL_GetError());

    SDL_EnableUNICODE(1);

    // Initialize data access and configuration.

    ::data = new app::data(DEFAULT_DATA_FILE);
    ::conf = new app::conf(DEFAULT_CONF_FILE);

    ::data->init();

    // Initialize language and host configuration.

    std::string lang_conf = ::conf->get_s("lang_file");
    std::string host_conf = ::conf->get_s("host_file");

    if (lang_conf.empty()) lang_conf = DEFAULT_LANG_FILE;
    if (host_conf.empty()) host_conf = DEFAULT_HOST_FILE;

    // If the given tag is an XML file name, use it as config file.
    // TODO: this is ugly

    if (tag.size() > 4 && tag.rfind(".xml") == tag.size() - 4)
        host_conf = tag;

    ::lang = new app::lang(lang_conf);
    ::host = new app::host(this, host_conf, exe, tag);

    // Initialize the OpenGL context.

    video();

    // Initialize the OpenGL state.

    ::user = new app::user();
    ::glob = new app::glob();
    ::perf = new app::perf();

    // Configure some application-level key bindings.

    key_snap = ::conf->get_i("key_snap");
    key_exit = ::conf->get_i("key_exit");
    key_init = ::conf->get_i("key_init");

    // Configure the joystick system.

    int j = ::conf->get_i("gamepad_device");

    if (SDL_JoystickOpen(j))
        SDL_JoystickEventState(SDL_ENABLE);

    // Initialize the input handler.

    std::string input_mode = ::conf->get_s("input_mode");

    if      (input_mode == "hybrid")  input = new dev::hybrid("hybrid.xml");
    else if (input_mode == "gamepad") input = new dev::gamepad();
    else if (input_mode == "trackd")  input = new dev::trackd();
//  else if (input_mode == "wiimote") input = new dev::wiimote();
    else                              input = new dev::mouse  ();
}

app::prog::~prog()
{
    // Release all resources

    if (input)  delete input;

    if (::perf) delete ::perf;
    if (::user) delete ::user;
    if (::host) delete ::host;
    if (::glob) delete ::glob;
    if (::lang) delete ::lang;
    if (::conf) delete ::conf;
    if (::data) delete ::data;

    SDL_Quit();

#ifdef _WIN32
    WSACleanup();
#endif
}

//-----------------------------------------------------------------------------

bool app::prog::process_event(app::event *E)
{
    // Give the input device an opportunity to translate the event.

    if (input && input->process_event(E))
        return true;

    // Otherwise, handle the global key bindings.

    else if (E->get_type() == E_KEY && E->data.key.d)
    {
        const int k = E->data.key.k;

        SDL_Event user = { SDL_USEREVENT };
        SDL_Event quit = { SDL_QUIT      };

        // Take a screenshot.

        if (k == key_snap)
        {
            screenshot(::conf->get_s("screenshot_file"),
                       ::host->get_window_w(),
                       ::host->get_window_h());

            return true;
        }

        // Exit or reload.

        else if (k == key_exit) { SDL_PushEvent(&quit); return true; }
        else if (k == key_init) { SDL_PushEvent(&user); return true; }
    }
    return false;
}

void app::prog::run()
{
    ::host->loop();
}

//-----------------------------------------------------------------------------

void app::prog::screenshot(std::string filename, int w, int h) const
{
    FILE       *filep  = NULL;
    png_structp writep = NULL;
    png_infop   infop  = NULL;
    png_bytep  *bytep  = NULL;

    // Initialize all PNG export data structures.

    if (!(filep = fopen(filename.c_str(), "wb")))
        throw std::runtime_error("Failure opening PNG file for writing");

    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG write structure");

    if (!(infop = png_create_info_struct(writep)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        unsigned char *p = NULL;

        // Initialize the PNG header.

        png_init_io (writep, filep);
//      png_set_compression_level(writep, 9);
        png_set_IHDR(writep, infop, w, h, 8, PNG_COLOR_TYPE_RGB,
                                             PNG_INTERLACE_NONE,
                                             PNG_COMPRESSION_TYPE_DEFAULT,
                                             PNG_FILTER_TYPE_DEFAULT);

        // Allocate the pixel buffer and copy pixels there.

        if ((p = new unsigned char[w * h * 3]))
        {
            glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

            // Allocate and initialize the row pointers.

            if ((bytep = (png_bytep *)png_malloc(writep, h*sizeof(png_bytep))))
            {
                for (int i = 0; i < h; ++i)
                    bytep[h - i - 1] = (png_bytep) (p + i * w * 3);

                // Write the PNG image file.

                png_set_rows  (writep, infop, bytep);
                png_write_info(writep, infop);
                png_write_png (writep, infop, 0, NULL);

                free(bytep);
            }
            else throw std::runtime_error("Failure allocating PNG row array");

            delete [] p;
        }
    }

    // Release all resources.

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);
}

//-----------------------------------------------------------------------------
