//  Copyright (C) 2007-2011 Robert Kooima
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
#include <cstring>
#include <algorithm>

#include <etc-socket.hpp>
#include <etc-math.hpp>

#include <ogl-range.hpp>
#include <ogl-frame.hpp>
#include <ogl-opengl.hpp>

#include <dpy-anaglyph.hpp>
#include <dpy-channel.hpp>
#include <dpy-direct.hpp>
#include <dpy-display.hpp>
#include <dpy-fulldome.hpp>
#include <dpy-interlace.hpp>
#include <dpy-lenticular.hpp>
#include <dpy-normal.hpp>
#include <dpy-oculus.hpp>

#include <app-default.hpp>
#include <app-frustum.hpp>
#include <app-event.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
#include <app-prog.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>

#define JIFFY (1.0 / 60.0)

//-----------------------------------------------------------------------------

#ifdef _WIN32
#define close _close
#endif

//-----------------------------------------------------------------------------

app::host::host(app::prog *p, std::string filename,
                              std::string exe,
                              std::string tag) :
    listen_sd(INVALID_SOCKET),
    script_sd(INVALID_SOCKET),
    server_sd(INVALID_SOCKET),
    clients(0),
    bench(::conf->get_i("bench")),
    movie(::conf->get_i("movie")),
    count(0),
    calibration_state(false),
    calibration_index(0),
    device(0),
    overlay(0),
    program(p),
    render(0),
    file(filename.c_str())
{
    // Set some reasonable defaults.

    window_full    = 0;
    window_frame   = 1;
    window_cursor  = 1;
    window_rect[0] = 0;
    window_rect[1] = 0;
    window_rect[2] = DEFAULT_PIXEL_WIDTH;
    window_rect[3] = DEFAULT_PIXEL_HEIGHT;
    buffer_size[0] = DEFAULT_PIXEL_WIDTH;
    buffer_size[1] = DEFAULT_PIXEL_HEIGHT;
    render_size[0] = 0;
    render_size[1] = 0;

    load_idt(T);

    // Read host.xml and configure using tag match.

    if (app::node p = file.get_root().find("host"))
    {
        // Extract the global tranform.

        if (app::node c = p.find("transform"))
        {
            T[ 0] = c.get_f("m0", 1.0);
            T[ 1] = c.get_f("m1", 0.0);
            T[ 2] = c.get_f("m2", 0.0);
            T[ 3] = c.get_f("m3", 0.0);
            T[ 4] = c.get_f("m4", 0.0);
            T[ 5] = c.get_f("m5", 1.0);
            T[ 6] = c.get_f("m6", 0.0);
            T[ 7] = c.get_f("m7", 0.0);
            T[ 8] = c.get_f("m8", 0.0);
            T[ 9] = c.get_f("m9", 0.0);
            T[10] = c.get_f("ma", 1.0);
            T[11] = c.get_f("mb", 0.0);
            T[12] = c.get_f("mc", 0.0);
            T[13] = c.get_f("md", 0.0);
            T[14] = c.get_f("me", 0.0);
            T[15] = c.get_f("mf", 1.0);
        }

        // Locate the configuration for this node.

        if (app::node n = p.find("node", "name", tag.c_str()))
        {
            // Extract the on-screen window configuration.

            if (app::node c = n.find("window"))
            {
                window_full    = c.get_i("full",   0);
                window_frame   = c.get_i("frame",  1);
                window_cursor  = c.get_i("cursor", 1);
                window_rect[0] = c.get_i("x", 0);
                window_rect[1] = c.get_i("y", 0);
                window_rect[2] = c.get_i("w", DEFAULT_PIXEL_WIDTH);
                window_rect[3] = c.get_i("h", DEFAULT_PIXEL_HEIGHT);
            }

            // Extract the off-screen render size, if any.

            if (app::node c = n.find("render"))
            {
                render_size[0] = c.get_i("w", 0);
                render_size[1] = c.get_i("h", 0);
            }

            // Extract the working buffer size, or use the window size.

            if (app::node c = n.find("buffer"))
            {
                buffer_size[0] = c.get_i("w", window_rect[2]);
                buffer_size[1] = c.get_i("h", window_rect[3]);
            }
            else
            {
                buffer_size[0] = window_rect[2];
                buffer_size[1] = window_rect[3];
            }

            // Extract the preferred CUDA device configuration.

            if (app::node c = n.find("device"))
                device = c.get_i("index");

            // Create a display object for each configured display.

            app::node c;

            for (c = n.find("display"); c; c = n.next(c, "display"))
            {
                const std::string t = c.get_s("type");

                if      (t == "anaglyph")
                    displays.push_back(new dpy::anaglyph  (c));
                else if (t == "fulldome")
                    displays.push_back(new dpy::fulldome  (c));
                else if (t == "interlace")
                    displays.push_back(new dpy::interlace (c));
                else if (t == "lenticular")
                    displays.push_back(new dpy::lenticular(c));
                else if (t == "normal")
                    displays.push_back(new dpy::normal    (c));
                else if (t == "oculus")
                    displays.push_back(new dpy::oculus    (c));
                else
                    displays.push_back(new dpy::direct    (c));
            }

            // Create a channel object for each configured channel.

            if (channels.empty())
                for (c = n.find("channel"); c; c = n.next(c, "channel"))
                    channels.push_back(new dpy::channel(c));

            // If there are no locally-defined channels, check the root.

            if (channels.empty())
                for (c = p.find("channel"); c; c = p.next(c, "channel"))
                    channels.push_back(new dpy::channel(c));

            // Determine the locally-defined overlay area.

            if (overlay == 0)
                if (app::node o = n.find("overlay"))
                {
                    int w = o.get_i("w", window_rect[2]);
                    int h = o.get_i("h", window_rect[3]);

                    if (app::node c = o.find("frustum"))
                        overlay = new app::frustum(c, w, h);
                    else
                        overlay = new app::frustum(0, w, h);
                }

            // Determine the globally-defined overlay area.

            if (overlay == 0)
                if (app::node o = p.find("overlay"))
                {
                    int w = o.get_i("w", window_rect[2]);
                    int h = o.get_i("h", window_rect[3]);

                    if (app::node c = o.find("frustum"))
                        overlay = new app::frustum(c, w, h);
                    else
                        overlay = new app::frustum(0, w, h);
                }

            // Start the network syncronization.

            init_server(n);
            init_client(n, exe);
            init_listen(n);
        }
    }
    init_script();

    // If no channels or displays were configured, instance defaults.

    if (channels.empty()) channels.push_back(new dpy::channel(0));
    if (displays.empty()) displays.push_back(new dpy::direct (0));

    // If no overlay has been defined, clone the first display frustum.

    if (overlay == 0)
        overlay = new app::frustum(*(displays.front()->get_overlay()));

    // Wait until all clients have connected.

    while (int(client_sd.size()) < clients)
        poll_listen(true);
}

app::host::~host()
{
    if (overlay)
        delete overlay;

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        delete (*i);

    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        delete (*i);

    if (render)
        delete render;

    fini_client();
    fini_server();
    fini_listen();
}

//-----------------------------------------------------------------------------

static void nodelay(int sd)
{
    socklen_t len = sizeof (int);
    int       val = 1;

    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (const char *) &val, len) < 0)
        throw std::runtime_error(strerror(sock_errno));
}

static in_addr_t lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)) == 0)
        throw app::host_error(hostname);

    memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);

    return A.s_addr;
}

static bool selectone(SOCKET sd, struct timeval *tv)
{
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(sd, &fds);

    int n = select(sd + 1, &fds, NULL, NULL, tv);

    if (n > 0)
    {
        if (FD_ISSET(sd, &fds))
            return true;
    }
    if (n < 0)
    {
        if (sock_errno != EINTR)
            throw app::sock_error("select");
    }
    return false;
}

//-----------------------------------------------------------------------------

SOCKET app::host::init_socket(int type, int port)
{
    SOCKET sd = INVALID_SOCKET;

    if (port)
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        address.sin_family      = AF_INET;
        address.sin_port        = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;

        // Create a socket.

        if ((sd = socket(AF_INET, type, 0)) == INVALID_SOCKET)
            throw std::runtime_error(strerror(sock_errno));

        // Bind the port.

        while (bind(sd, (struct sockaddr *) &address, addresslen) < 0)
            if (sock_errno == EINVAL)
            {
                fprintf(stderr, "Waiting for port expiration\n");
                usleep(250000);
            }
            else throw std::runtime_error(strerror(sock_errno));

        // If this is a TCP socket, set nodelay and listen.

        if (type == SOCK_STREAM)
        {
            nodelay(sd);
            listen(sd, 16);
        }
    }
    return sd;
}

//-----------------------------------------------------------------------------

void app::host::init_listen(app::node p)
{
    if (clients) listen_sd = init_socket(SOCK_STREAM, p.get_i("port"));
}

void app::host::poll_listen(bool wait)
{
    struct timeval tv = { 0, 0 };

    if (listen_sd != INVALID_SOCKET)
    {
        if (selectone(listen_sd, wait ? 0 : &tv))
        {
            // Accept any incoming client connection.

            if (int sd = accept(listen_sd, 0, 0))
            {
                if (sd < 0)
                    throw app::sock_error("accept");
                else
                {
                    nodelay(sd);
                    client_sd.push_back(sd);
                }
            }
        }
    }
}

void app::host::fini_listen()
{
    close(listen_sd);
    listen_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_script()
{
    if (root()) script_sd = init_socket(SOCK_DGRAM, DEFAULT_SCRIPT_PORT);
}

void app::host::poll_script()
{
    struct timeval tv = { 0, 0 };

    if (script_sd != INVALID_SOCKET)
    {
        if (selectone(script_sd, &tv))
        {
            char    buf[256];
            ssize_t len;
            long long l;

            memset(buf, 0, sizeof (buf));

            if ((len = ::recv(script_sd, buf, sizeof (buf), 0)) > 0)
            {
                printf("recv \"%s\" on port %d\n", buf, DEFAULT_SCRIPT_PORT);

                event E;
                memcpy(&l, buf, sizeof (l));
                process_event(E.mk_user(l));
            }
        }
    }
}

void app::host::fini_script()
{
    close(script_sd);
    script_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_server(app::node p)
{
    // If we have a server assignment then we must connect to it.

    if (app::node n = p.find("server"))
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        std::string addr = n.get_s("addr");
        int         port = n.get_i("port");

        if (addr.empty()) addr = DEFAULT_HOST;
        if (port == 0)    port = DEFAULT_PORT;

        // Look up the given host name.

        address.sin_family      = AF_INET;
        address.sin_port        = htons (port);
        address.sin_addr.s_addr = lookup(addr.c_str());

        // Create a socket and connect.

        if ((server_sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            throw app::sock_error(addr);

        while (connect(server_sd, (struct sockaddr *) &address, addresslen) <0)
            if (sock_errno == ECONNREFUSED)
            {
                fprintf(stderr, "Waiting for %s\n", addr.c_str());
                usleep(250000);
            }
            else throw app::sock_error(addr);

        nodelay(server_sd);
    }
}

void app::host::fini_server()
{
    if (server_sd != INVALID_SOCKET)
        close(server_sd);

    server_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_client(app::node p, const std::string& exe)
{
    // Launch all client processes.

    for (app::node n = p.find("client"); n; n = p.next(n, "client"))
    {
        fork_client(n.get_s("name").c_str(),
                    n.get_s("addr").c_str(),
                    n.get_s("disp").c_str(),
                                exe.c_str());
        clients++;
    }
}

void app::host::fini_client()
{
    while (!client_sd.empty())
    {
        int sd = client_sd.front();
        char c;

        // Wait for EOF (orderly shutdown by the remote).

        while (::recv(sd, &c, 1, 0) > 0)
            ;

        // Close the socket.

        close(sd);

        client_sd.pop_front();
    }
}

void app::host::fork_client(const char *name,
                            const char *addr,
                            const char *disp,
                            const char *exe)
{
#ifndef _WIN32 // W32 HACK
    const char *cmd = "/bin/sh -c 'cd %s; DISPLAY=%s %s %s'";

    char *cwd = getenv("PWD");

    char str[256];

    if (disp && strlen(disp))
        sprintf(str, cmd, cwd,   disp, exe, name);
    else
        sprintf(str, cmd, cwd, ":0.0", exe, name);

    fprintf(stderr, "%s\n", str);

    // Fork and build the client's ssh command line.

    if ((fork() == 0))
    {
        const char *args[4];

        args[0] = "ssh";
        args[1] = addr;
        args[2] = str;
        args[3] = NULL;

        execvp("ssh", (char * const *) args);

        exit(0);
    }
#endif
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    event E;

    // Kick off with a START event.

    process_event(E.mk_start());

    // Process incoming events until an exit is posted.

    while (program->is_running())
    {
        // Translate and dispatch SDL events.

        SDL_Event e;

        while (program->is_running() && SDL_PollEvent(&e))
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                if (pointer_to_3D(&E, e.motion.x, window_rect[3] - e.motion.y))
                    process_event(&E);
                break;

            case SDL_MOUSEBUTTONDOWN:
                process_event(E.mk_click(e.button.button,
                                         SDL_GetModState(), true));
                break;

            case SDL_MOUSEBUTTONUP:
                process_event(E.mk_click(e.button.button,
                                         SDL_GetModState(), false));
                break;

            case SDL_KEYDOWN:
                if (e.key.repeat == 0)
                    process_event(E.mk_key(e.key.keysym.scancode,
                                           SDL_GetModState(), true));
                break;

            case SDL_KEYUP:
                if (e.key.repeat == 0)
                    process_event(E.mk_key(e.key.keysym.scancode,
                                           SDL_GetModState(), false));
                break;

            case SDL_TEXTINPUT:
                process_event(E.mk_text(e.text.text[0]));
                break;

            case SDL_JOYAXISMOTION:
                process_event(program->axis_remap(E.mk_axis(e.jaxis.which,
                                                            e.jaxis.axis,
                                                            e.jaxis.value)));
                break;

            case SDL_JOYBUTTONDOWN:
                process_event(E.mk_button(e.jbutton.which,
                                          e.jbutton.button, true));
                break;

            case SDL_JOYBUTTONUP:
                process_event(E.mk_button(e.jbutton.which,
                                          e.jbutton.button, false));
                break;

            case SDL_USEREVENT:
                process_event(E.mk_flush());
                break;

            case SDL_QUIT:
                process_event(E.mk_close());
                break;
            }

        if (program->is_running())
        {
            poll_listen(false);
            poll_script();

            // Advance to the current time, or by one JIFFY when benchmarking.

            if (bench || movie)
                process_event(E.mk_tick(JIFFY));
            else
                for (double tick = SDL_GetTicks() / 1000.0;
                            tick - tock >= JIFFY;
                            tock        += JIFFY)
                    process_event(E.mk_tick(JIFFY));

            // Call the render handler.

            process_event(E.mk_draw());
            process_event(E.mk_swap());

            // Count frames and record a movie, if requested.

            if (movie)
            {
                count++;

                if ((count % movie) == 0)
                {
                    char buf[256];

                    sprintf(buf, "frame%06d.tga", count / movie);

                    if (render)
                    {
                        render->bind();
                        program->screenshot(std::string(buf),
                                            render->get_w(),
                                            render->get_h());
                        render->free();
                    }
                    else
                        program->screenshot(std::string(buf),
                                            get_window_w(),
                                            get_window_h());

                }
            }
        }
    }
}

void app::host::node_loop()
{
    event E;

    while (program->is_running())
        process_event(E.recv(server_sd));
}

void app::host::loop()
{
    if (root())
        root_loop();
    else
        node_loop();
}

//-----------------------------------------------------------------------------

void app::host::draw()
{
    // Instance the off-screen render buffer, if needed.

    if (render == 0 && render_size[0] && render_size[1])
        render = new ogl::frame(render_size[0], render_size[1],
                                GL_TEXTURE_RECTANGLE_ARB,
                                GL_RGBA, true, true, false);

    // Channel and frustum vectors are passed C-style.

    const dpy::channel *const *chanv = &channels.front();
    const app::frustum *const *frusv = &frustums.front();

    int chanc = int(channels.size());
    int frusc = int(frustums.size());
    int frusi = 0;

    // Prepare all displays for rendering (cheap).

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->prep(chanc, chanv);

    // Cache the transformed frustum planes (cheap).

    for (app::frustum_i i = frustums.begin(); i != frustums.end(); ++i)
        (*i)->set_transform(::view->get_view_matrix());

    // Determine visibility and view distance (moderately expensive).

    ogl::range r = program->prep(frusc, frusv);

    // Cache the frustum projections (cheap).

    for (app::frustum_i i = frustums.begin(); i != frustums.end(); ++i)
        (*i)->set_distances(r.get_n(), r.get_f());

    // Perform the lighting prepass (possibly expensive).

    program->lite(frusc, frusv);

    // Update all modified uniforms.

    ::glob->prep();

    // Switch to off-screen if necessary.

    if (render)
        render->bind();

    // Clear the entire window.

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    // Render all displays (probably very expensive).

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
    {
        if (calibration_state)
            (*i)->test(chanc, chanv, calibration_index);
        else
            (*i)->draw(chanc, chanv, frusi);

        frusi += (*i)->get_frusc();
    }

    // Switch to on-screen if necessary.

    if (render)
    {
        render->free();
        render->draw();
    }
}

void app::host::draw(int frusi, const app::frustum *frusp, int chani)
{
    program->draw(frusi, frusp, chani);
}

void app::host::swap() const
{
    // If doing network sync, wait until the rendering has finished.

    if (server_sd != INVALID_SOCKET || !client_sd.empty())
        glFinish();

    program->swap();
}

//-----------------------------------------------------------------------------

// Send the given event to all connected clients.

void app::host::send(event *E)
{
    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        E->send(*i);
}

// Barrier-sync. Await an acknowledgement from all connected clients and
// send an acknowledgement to the server. This has the effect of a tree-
// wide recursive barrier synchronization.

void app::host::sync()
{
    char buf[1] = { '\0' };

    // Recieve a message from all connected clients.

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        ::recv(*i,        buf, 1, 0);

    // Send a message to any connected server.

    if (server_sd != INVALID_SOCKET)
        ::send(server_sd, buf, 1, 0);
}

//-----------------------------------------------------------------------------

bool app::host::process_calib(event *E)
{
    // Handle calibration state input.

    if ((E->get_type() == E_KEY) &&
        (E->data.key.d)          &&
        (E->data.key.m & KMOD_CTRL))

        switch (E->data.key.k)
        {
        case SDL_SCANCODE_TAB:
            calibration_state = !calibration_state;
            return true;

        case SDL_SCANCODE_SPACE:
            calibration_index++;
            printf("calibrating index %d\n", calibration_index);
            return true;

        case SDL_SCANCODE_BACKSPACE:
            calibration_index--;
            printf("calibrating index %d\n", calibration_index);
            return true;
        }

    // Dispatch a calibration event to the current display.

    if (calibration_state)
        for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
            if ((*i)->is_index(calibration_index))
                return (*i)->process_event(E);

    return false;
}

void app::host::process_start(event *E)
{
    // Forward the start event to all channels and displays.

    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        (*i)->process_event(E);
    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->process_event(E);

    // Determine the total number of display frustums.

    int frusc = 0;
    int frusi = 0;

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        frusc += (*i)->get_frusc();

    frustums.resize(frusc);

    // Make a list of all display frustums.

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
    {
                 (*i)->get_frusv(&frustums[frusi]);
        frusi += (*i)->get_frusc();
    }

    // Start the timer.

    tock = SDL_GetTicks() / 1000.0;
}

void app::host::process_close(event *E)
{
    program->stop();

    // Free the list of display frustums.

    frustums.clear();

    // Forward the close event to all channels and displays.

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->process_event(E);
    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        (*i)->process_event(E);
}

// Handle the given user event.

bool app::host::process_event(event *E)
{
    // Forward the message to all clients.

    send(E);

    // Allow the application or the calibration to process the event.

    if (program->process_event(E)
        ||       process_calib(E))
        return true;

    // Handle the event locally, as needed.

    switch (E->get_type())
    {
    case E_DRAW:  draw();                   return true;
    case E_SWAP:  sync(); swap();           return true;
    case E_START: sync(); process_start(E); return true;
    case E_CLOSE: process_close(E); sync(); return true;
    case E_FLUSH: ::glob->fini();
                  ::glob->init(); return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

// Ask each display to project the event into the virtual space.

bool app::host::pointer_to_3D(event *E, int x, int y)
{
    if (render_size[0] || render_size[1])
    {
        x = x * render_size[0] / window_rect[2];
        y = y * render_size[1] / window_rect[3];
    }

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        if ((*i)->pointer_to_3D(E, x, y))
            return true;

    return false;
}

int app::host::get_window_m() const
{
    return ((window_full  ? SDL_WINDOW_FULLSCREEN : 0) |
            (window_frame ? 0 : SDL_WINDOW_BORDERLESS));
}

// Forward the given position and orientation to all channel objects.  This is
// usually called per-frame in response to a head motion tracker input.

void app::host::set_head(const double *p,
                         const double *q)
{
    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        (*i)->set_head(p, q);
}

void app::host::navigate(const mat4& M)
{
    program->navigate(M);
}

vec3 app::host::get_up_vector() const
{
    return program->get_up_vector();
}

//-----------------------------------------------------------------------------
