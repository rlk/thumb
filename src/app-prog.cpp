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

#ifdef _WIN32
#include <io.h>
#define open  _open
#define write _write
#define close _close
#endif

#include <ogl-opengl.hpp>
#include <app-event.hpp>
#include <etc-vector.hpp>
#include <etc-log.hpp>

#include <app-prog.hpp>
#include <app-conf.hpp>
#include <app-data.hpp>
#include <app-glob.hpp>
#include <app-view.hpp>
#include <app-lang.hpp>
#include <app-host.hpp>
#include <app-perf.hpp>

#include <dev-mouse.hpp>
#include <dev-hybrid.hpp>
#include <dev-sixense.hpp>
#include <dev-skeleton.hpp>
#include <dev-gamepad.hpp>
#include <dev-trackd.hpp>

//-----------------------------------------------------------------------------
// Global application state

app::conf *conf = 0;
app::data *data = 0;
app::glob *glob = 0;
app::view *view = 0;
app::lang *lang = 0;
app::host *host = 0;
app::perf *perf = 0;

//-----------------------------------------------------------------------------

#define XTR(S) #S
#define STR(S) XTR(S)

static const char *version = "Thumb Version " STR(VERSION);

//-----------------------------------------------------------------------------

void app::prog::video_up()
{
    // Look up the video mode parameters.

    int m = ::host->get_window_m() | SDL_WINDOW_OPENGL;
    int x = ::host->get_window_x();
    int y = ::host->get_window_y();
    int w = ::host->get_window_w();
    int h = ::host->get_window_h();
    int c = ::host->get_window_c();

    // Look up the GL context parameters.

    int mults = ::conf->get_i("multisample_samples");
    int multb = ::conf->get_i("multisample_buffers");
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

        if ((window = SDL_CreateWindow(version, x, y, w, h, m)))
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

    SDL_ShowCursor           (c ? SDL_ENABLE : SDL_DISABLE);
    SDL_SetWindowGrab(window, c ? SDL_FALSE  : SDL_TRUE);

    // Initialize the OpenGL state.

    context = SDL_GL_CreateContext(window);

    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &multb);
    SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &mults);

    SDL_GL_SetSwapInterval(::conf->get_i("sync", 1));

    glewInit();

    ogl::init(multb > 0 && mults > 0);
}

void app::prog::video_dn()
{
    SDL_SetWindowGrab(window, SDL_FALSE);

    ogl::fini();

    if (context) SDL_GL_DeleteContext(context);
    if (window)  SDL_DestroyWindow(window);

    context = 0;
    window  = 0;
}

void app::prog::host_up(std::string config)
{
    ::host = new app::host(this, config, exe, "default");

    video_up();

    ::perf = new app::perf(window);
    ::glob->init();
}

void app::prog::host_dn()
{
    ::glob->fini();

    video_dn();

    delete ::perf;
    delete ::host;

    ::perf = 0;
    ::host = 0;
}

//-----------------------------------------------------------------------------

app::prog::prog(const std::string& exe,
                const std::string& tag)
    : exe(exe), running(false), restart(false), input(0), snap_p(0), snap_w(0), snap_h(0)
{
    // Start Winsock

#ifdef _WIN32
    WSADATA wsadata;
    WSAStartup(0x202, &wsadata);
#endif

    // Start SDL

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK))
        throw std::runtime_error(SDL_GetError());

    // Initialize data access and configuration.

    ::data = new app::data(DEFAULT_DATA_FILE);
    ::conf = new app::conf(DEFAULT_OPTIONS_FILE);
    ::view = new app::view();

    ::data->init();

    // Initialize the input handlers.

    std::string input_mode = ::conf->get_s("input_mode");

    if      (input_mode == "gamepad")  input = new dev::gamepad();
    else if (input_mode == "skeleton") input = new dev::skeleton();
#ifdef WITH_SIXENSE
    else if (input_mode == "sixense")  input = new dev::sixense();
#endif
//  else if (input_mode == "hybrid")   input = new dev::hybrid("hybrid.xml");
//  else if (input_mode == "trackd")   input = new dev::trackd();
//  else if (input_mode == "hybrid")   input = new dev::hybrid("hybrid.xml");

    mouse = new dev::mouse();

    // Initialize language and host configuration.

    std::string lang_config;

    lang_config = ::conf->get_s("language_file");
    host_config = ::conf->get_s("config_file");

    if (lang_config.empty()) lang_config = DEFAULT_LANGUAGE_FILE;
    if (host_config.empty()) host_config = DEFAULT_CONFIG_FILE;

    ::lang = new app::lang(lang_config);
    ::glob = new app::glob();

    // Configure some application-level key bindings.

    key_init = ::conf->get_i("key_init", SDL_SCANCODE_F12);
    key_snap = ::conf->get_i("key_snap", SDL_SCANCODE_F11);

    SDL_StopTextInput();

    // Configure the joystick system.

    axis_setup();
}

app::prog::~prog()
{
    // Release all resources

    delete snap_p;

    if (mouse)  delete mouse;
    if (input)  delete input;

    if (::perf) delete ::perf;
    if (::view) delete ::view;
    if (::host) delete ::host;
    if (::glob) delete ::glob;
    if (::lang) delete ::lang;
    if (::conf) delete ::conf;
    if (::data) delete ::data;

    video_dn();
    SDL_Quit();

#ifdef _WIN32
    WSACleanup();
#endif
}

void app::prog::set_host_config(std::string config)
{
    SDL_Event quit = { SDL_QUIT };

    if (!config.empty()) host_config = config;
    restart = true;
    SDL_PushEvent(&quit);
}

//-----------------------------------------------------------------------------

bool app::prog::process_event(app::event *E)
{
    // Give the input device an opportunity to translate the event.

    if (input && input->process_event(E)) return true;
    if (mouse && mouse->process_event(E)) return true;

    // Otherwise, handle the global key bindings.

    else if (E->get_type() == E_KEY && E->data.key.d)
    {
        if (E->data.key.k == key_snap)
        {
            ::host->set_movie_mode(-1);
            return true;
        }
        if (E->data.key.k == key_init)
        {
            SDL_Event user = { SDL_USEREVENT };
            SDL_PushEvent(&user);
            return true;
        }
    }
    else if (E->get_type() == E_TICK)
        axis_state();

    return false;
}

void app::prog::run()
{
    host_up(host_config);

    for (;;)
    {
        restart = false;
        running = true;

        ::host->loop();

        if (restart)
        {
            host_dn();
            host_up(host_config);
        }
        else break;
    }

    host_dn();
}

void app::prog::stop()
{
    running = false;
}

void app::prog::swap()
{
    SDL_GL_SwapWindow(window);
}

void app::prog::dump(std::string name)
{
    unsigned char *p = 0;
    size_t         n = 0;

    if ((p = (unsigned char *) ::data->load(name, &n)))
    {
        for (size_t i = 0; i < n; i++)
            putchar(int(p[i]));
    }
}

//-----------------------------------------------------------------------------

// These functions permit the application to take control over changes to the
// view requested by the device. This enables context-sensitive interaction,
// domain-specific constraints, and coordinate system manipulation. The default
// it to pass requests along to the view without modification.

quat app::prog::get_orientation() const
{
    return ::view->get_orientation();
}

void app::prog::set_orientation(const quat &q)
{
    ::view->set_orientation(q);
}

void app::prog::offset_position(const vec3 &d)
{
    ::view->set_position(::view->get_position() + d);
}

//-----------------------------------------------------------------------------

#ifndef WIN32
#include <unistd.h>
#endif
#include <fcntl.h>

#pragma pack(push, 1)
struct tga
{
    unsigned char  image_id_length;
    unsigned char  color_map_type;
    unsigned char  image_type;
    unsigned short color_map_first_index;
    unsigned short color_map_length;
    unsigned char  color_map_entry_size;
    unsigned short image_x_origin;
    unsigned short image_y_origin;
    unsigned short image_width;
    unsigned short image_height;
    unsigned char  image_depth;
    unsigned char  image_descriptor;
};
#pragma pack(pop)

static void snaptga(const char *filename, unsigned char *p, int w, int h)
{
    size_t s = sizeof (tga);
    tga   *t = (tga *) p;
    int    d;

    t->image_id_length       =  0;
    t->color_map_type        =  0;
    t->image_type            =  2;
    t->color_map_first_index =  0;
    t->color_map_length      =  0;
    t->color_map_entry_size  =  0;
    t->image_x_origin        =  0;
    t->image_y_origin        =  0;
    t->image_width           =  w;
    t->image_height          =  h;
    t->image_depth           = 24;
    t->image_descriptor      =  0;

    glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, p + s);

    if ((d = open(filename, O_WRONLY | O_CREAT, 0666)) != -1)
    {
        if (write(d, p, s + w * h * 3) == -1)
            throw app::write_error(filename);

        close(d);
    }
}

//-----------------------------------------------------------------------------

static void snappng(const char *filename, unsigned char *p, int w, int h)
{
    FILE       *filep  = NULL;
    png_structp writep = NULL;
    png_infop   infop  = NULL;
    png_bytep  *bytep  = NULL;

    // Copy the frame buffer to the snap buffer.

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

    // Initialize all PNG export data structures.

    if (!(filep = fopen(filename, "wb")))
        throw std::runtime_error("Failure opening PNG file for writing");

    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        throw std::runtime_error("Failure creating PNG write structure");

    if (!(infop = png_create_info_struct(writep)))
        throw std::runtime_error("Failure creating PNG info structure");

    // Enable the default PNG error handler.

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        // Initialize the PNG header.

        png_init_io (writep, filep);
        png_set_compression_level(writep, 9);
        png_set_IHDR(writep, infop, w, h, 8, PNG_COLOR_TYPE_RGB,
                                             PNG_INTERLACE_NONE,
                                             PNG_COMPRESSION_TYPE_DEFAULT,
                                             PNG_FILTER_TYPE_DEFAULT);

        // Allocate and initialize the row pointers.

        if ((bytep = (png_bytep *) png_malloc(writep, h * sizeof(png_bytep))))
        {
            for (int i = 0; i < h; ++i)
                bytep[h - i - 1] = (png_bytep) (p + i * w * 3);

            // Write the PNG image file.

            png_set_rows  (writep, infop, bytep);
            png_write_info(writep, infop);
            png_write_png (writep, infop, 0, NULL);

            png_free(writep, bytep);
        }
        else throw std::runtime_error("Failure allocating PNG row array");
    }

    // Release all resources.

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);
}

//-----------------------------------------------------------------------------

static void snapraw(const char *filename, unsigned char *p, int w, int h)
{
    int d;

    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, p);

    if ((d = open(filename, O_WRONLY | O_CREAT, 0666)) != -1)
    {
        if (write(d, p, w * h * 3) == -1)
            throw app::write_error(filename);

        close(d);
    }
}

void app::prog::screenshot(std::string filename, int w, int h)
{
    if (snap_p == 0 || snap_w != w || snap_h != h)
    {
        delete [] snap_p;

        snap_p = new unsigned char[w * h * 3 + sizeof (tga)];
        snap_w = w;
        snap_h = h;
    }

    if (snap_p)
    {
        if      (filename.compare(filename.length() - 4, 4, ".png") == 0)
            snappng(filename.c_str(), snap_p, snap_w, snap_h);
        else if (filename.compare(filename.length() - 4, 4, ".tga") == 0)
            snaptga(filename.c_str(), snap_p, snap_w, snap_h);
        else
            snapraw(filename.c_str(), snap_p, snap_w, snap_h);

        etc::log("Image saved to %s", filename.c_str());
    }
}

//-----------------------------------------------------------------------------

// Apply the configured remapping of the open joystick device.

app::event *app::prog::axis_remap(app::event *E)
{
    const int a = E->data.axis.a;

    if (axis_max[a] != axis_min[a])

        E->data.axis.v = 65535.0 * (E->data.axis.v - axis_min[a])
                                 / (axis_max[a]    - axis_min[a]) - 32768.0;

    return E;
}

// Initialize the requested joystick device and its configuration.

void app::prog::axis_setup()
{
    if ((joystick = SDL_JoystickOpen(::conf->get_i("joystick_device", 0))))
    {
        const int n = SDL_JoystickNumAxes(joystick);

        axis_min.resize(n);
        axis_max.resize(n);

        char min[32];
        char max[32];

        for (int a = 0; a < n; ++a)
        {
            sprintf(min, "joystick_axis_min_%d", a);
            sprintf(max, "joystick_axis_max_%d", a);

            axis_min[a] = ::conf->get_i(min, -32768);
            axis_max[a] = ::conf->get_i(max,  32767);
        }

        SDL_JoystickEventState(SDL_ENABLE);
    }
    axis_verbose = ::conf->get_i("joystick_verbose", 0) ? true : false;
}

// Report the current state of the open joystick device.

void app::prog::axis_state()
{
    if (axis_verbose)
    {
        for (int i = 0; i < SDL_JoystickNumAxes   (joystick); ++i)
            fprintf(stderr, "%+7d",  SDL_JoystickGetAxis   (joystick, i));

        for (int i = 0; i < SDL_JoystickNumButtons(joystick); ++i)
            fprintf(stderr, "%2d",   SDL_JoystickGetButton (joystick, i));

        fprintf(stderr, "\n");
    }
}

//-----------------------------------------------------------------------------
