//  Copyright (C) 2007 Robert Kooima
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
#include <iostream>

#include <string.h>

#include "util.hpp"
#include "matrix.hpp"
#include "default.hpp"
#include "ogl-range.hpp"
#include "ogl-opengl.hpp"
#include "dpy-channel.hpp"
#include "dpy-display.hpp"
#include "dpy-normal.hpp"
#include "dpy-anaglyph.hpp"
#include "dpy-lenticular.hpp"
#include "app-frustum.hpp"
#include "app-event.hpp"
#include "app-conf.hpp"
#include "app-user.hpp"
#include "app-prog.hpp"
#include "app-glob.hpp"
#include "app-host.hpp"

#define JIFFY (1000 / 60)

//-----------------------------------------------------------------------------

static unsigned long lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)) == 0)
        throw app::host_error(hostname);

    memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);

    return A.s_addr;
}

//-----------------------------------------------------------------------------

void app::host::fork_client(const char *name,
                            const char *addr,
                            const char *disp)
{
#ifndef _WIN32 // W32 HACK
    const char *args[4];
    char line[256];

    if ((fork() == 0))
    {
        std::string dir = ::conf->get_s("exec_dir");

        sprintf(line, "/bin/sh -c 'cd %s; DISPLAY=%s ./thumb %s'",
                dir.c_str(), disp ? disp : ":0.0", name);

        // Allocate and build the client's ssh command line.

        args[0] = "ssh";
        args[1] = addr;
        args[2] = line;
        args[3] = NULL;

        execvp("ssh", (char * const *) args);

        exit(0);
    }
#endif
}

//-----------------------------------------------------------------------------

SOCKET app::host::init_socket(int port)
{
    int sd = INVALID_SOCKET;

    if (port)
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        address.sin_family      = AF_INET;
        address.sin_port        = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;

        // Create a socket, set no-delay, bind the port, and listen.

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
            throw std::runtime_error(strerror(sock_errno));
        
        nodelay(sd);

        while (bind(sd, (struct sockaddr *) &address, addresslen) < 0)
            if (sock_errno == EINVAL)
            {
                std::cerr << "Waiting for port expiration" << std::endl;
                usleep(250000);
            }
            else throw std::runtime_error(strerror(sock_errno));

        listen(sd, 16);
    }
    return sd;
}

void app::host::init_listen(app::node node)
{
    if (clients) client_cd = init_socket(get_attr_d(node, "port"));
    if (root())  script_cd = init_socket(DEFAULT_SCRIPT_PORT);
}

void app::host::poll_listen()
{
    struct timeval tv = { 0, 0 };

    fd_set fds;
        
    FD_ZERO(&fds);

    if (client_cd != INVALID_SOCKET) FD_SET(client_cd, &fds);
    if (script_cd != INVALID_SOCKET) FD_SET(script_cd, &fds);

    int m = std::max(client_cd, script_cd);

    // Check for an incomming client connection.

    if (int n = select(m + 1, &fds, NULL, NULL, &tv))
    {
        if (n < 0)
        {
            if (sock_errno != EINTR)
                throw app::sock_error("select");
        }
        else
        {
            // Accept any client connection.

            if (client_cd != INVALID_SOCKET && FD_ISSET(client_cd, &fds))
            {
                if (int sd = accept(client_cd, 0, 0))
                {
                    if (sd < 0)
                        throw app::sock_error("accept");
                    else
                    {
                        // HACK to generate a start event
                        event E;
                        E.mk_start();
                        nodelay(sd);
                        E.send(sd);
                        client_sd.push_back(sd);
                    }
                }
            }

            // Accept any script connection.

            if (script_cd != INVALID_SOCKET && FD_ISSET(script_cd, &fds))
            {
                if (int sd = accept(script_cd, 0, 0))
                {
                    if (sd < 0)
                        throw app::sock_error("accept");
                    else
                    {
                        nodelay(sd);
                        script_sd.push_back(sd);
                        printf("script socket connected\n");
                    }
                }
            }
        }
    }
}

void app::host::fini_listen()
{
    if (client_cd != INVALID_SOCKET) ::close(client_cd);
    if (script_cd != INVALID_SOCKET) ::close(script_cd);

    client_cd = INVALID_SOCKET;
    script_cd = INVALID_SOCKET;
}

void app::host::poll_script()
{
    struct timeval tv = { 0, 0 };

    fd_set fds;
    int m = 0;

    // Initialize the socket descriptor set with all connected sockets.

    FD_ZERO(&fds);

    for (SOCKET_i i = script_sd.begin(); i != script_sd.end(); ++i)
    {
        FD_SET(*i, &fds);
        m = std::max(m, *i);
    }

    // Check for activity on all sockets.

    if (int n = select(m + 1, &fds, NULL, NULL, &tv))
    {
        if (n < 0)
        {
            if (sock_errno != EINTR)
                throw std::runtime_error(strerror(sock_errno));
        }
        else
        {
            for (SOCKET_i t, i = script_sd.begin(); i != script_sd.end(); i = t)
            {
                // Step lightly in case *i is removed from the vector.

                t = i;
                t++;

                // Check for and handle script input.

                if (FD_ISSET(*i, &fds))
                {
                    char ibuf[256];
                    char obuf[256];

                    ssize_t sz;

                    memset(ibuf, 0, 256);
                    memset(obuf, 0, 256);

                    if ((sz = ::recv(*i, ibuf, 256, 0)) <= 0)
                    {
                        printf("script socket disconnected\n");
                        ::close(*i);
                        script_sd.erase(i);
                    }
                    else
                    {
                        // Process the command and return any result.

                        printf("script socket received [%s]\n", ibuf);

//                      messg(ibuf, obuf);
                        sz = strlen(obuf);

                        if (sz > 0) ::send(*i, obuf, sz, 0);
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

void app::host::init_server(app::node node)
{
    // If we have a server assignment then we must connect to it.

    if (app::node server = find(node, "server"))
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        std::string addr = get_attr_s(server, "addr");
        int         port = get_attr_d(server, "port");

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
                std::cerr << "Waiting for " << addr << std::endl;
                usleep(250000);
            }
            else throw app::sock_error(addr);

        nodelay(server_sd);
    }
}

void app::host::fini_server()
{
    if (server_sd != INVALID_SOCKET)
        ::close(server_sd);

    server_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_client(app::node node)
{
    app::node curr;

    // Launch all client processes.

    for (curr = find(node,       "client"); curr;
         curr = next(node, curr, "client"))
    {
        fork_client(get_attr_s(curr, "name").c_str(),
                    get_attr_s(curr, "addr").c_str(),
                    get_attr_s(curr, "disp").c_str());
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

        ::close(sd);

        client_sd.pop_front();
    }
}

void app::host::fini_script()
{
    while (!script_sd.empty())
    {
        int sd = script_sd.front();
        char c;

        // Wait for EOF (orderly shutdown by the remote).

        while (::recv(sd, &c, 1, 0) > 0)
            ;

        // Close the socket.

        ::close(sd);

        script_sd.pop_front();
    }
}

//-----------------------------------------------------------------------------

bool app::host::process_calib(event *E)
{
    // Handle calibration state input.

    if ((E->get_type() == E_KEYBD) &&
        (E->data.keybd.d)          &&
        (E->data.keybd.m & KMOD_CTRL))

        switch (E->data.keybd.k)
        {
        case SDLK_TAB:
            calibration_state = !calibration_state;
            post_draw();
            return true;

        case SDLK_SPACE:
            calibration_index++;
            post_draw();
            return true;

        case SDLK_BACKSPACE:
            calibration_index--;
            post_draw();
            return true;
        }

    // Dispatch a calibration event to the current display.

    if (calibration_state)
        for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
            if ((*i)->is_index(calibration_index))
                return (*i)->process_event(E);

    return false;
}

//-----------------------------------------------------------------------------

app::host::host(std::string filename, std::string tag) :
    clients(0),
    server_sd(INVALID_SOCKET),
    client_cd(INVALID_SOCKET),
    script_cd(INVALID_SOCKET),
    draw_flag(false),
    exit_flag(false),
    bench(0),
    movie(0),
    count(0),
    calibration_state(false),
    calibration_index(0),
    overlay(0),
    file(filename.c_str())
{
    // Set some reasonable defaults.

    window_full    = 0;
    window_frame   = 1;
    window_size[0] = 0;
    window_size[1] = 0;
    window_size[2] = DEFAULT_PIXEL_WIDTH;
    window_size[3] = DEFAULT_PIXEL_HEIGHT;
    buffer_size[0] = DEFAULT_PIXEL_WIDTH;
    buffer_size[1] = DEFAULT_PIXEL_HEIGHT;

    // Read host.xml and configure using tag match.

    app::node root;
    app::node node;
    app::node curr;

    if ((root = find(file.get_head(), "host")))
    {
        // Locate the configuration for this node.

        if ((node = find(root, "node", "name", tag.c_str())))
        {
            // Extract the on-screen window configuration.

            if (app::node win = find(node, "window"))
            {
                window_full    = get_attr_d(win, "full",  0);
                window_frame   = get_attr_d(win, "frame", 1);
                window_size[0] = get_attr_d(win, "x", 0);
                window_size[1] = get_attr_d(win, "y", 0);
                window_size[2] = get_attr_d(win, "w", DEFAULT_PIXEL_WIDTH);
                window_size[3] = get_attr_d(win, "h", DEFAULT_PIXEL_HEIGHT);
            }

            // Extract the off-screen buffer size, or use the window size.

            if (app::node buf = find(node, "buffer"))
            {
                buffer_size[0] = get_attr_d(buf, "w", window_size[2]);
                buffer_size[1] = get_attr_d(buf, "h", window_size[3]);
            }

            // Create a display object for each configured display.

            for (curr = find(node,       "display"); curr;
                 curr = next(node, curr, "display"))
            {
                const std::string t = get_attr_s(curr, "type");

                if      (t == "anaglyph")
                    displays.push_back(new dpy::anaglyph  (curr));
                else if (t == "lenticular")
                    displays.push_back(new dpy::lenticular(curr));
//              else if (t == "dome")
//                  displays.push_back(new dpy::dome      (curr));
//              else if (t == "varrier")
//                  displays.push_back(new dpy::varrier   (curr));
                else
                    displays.push_back(new dpy::normal    (curr));
            }

            // Create a channel object for each configured channel.

            if (channels.empty())
                for (curr = find(node,       "channel"); curr;
                     curr = next(node, curr, "channel"))
                    channels.push_back(new dpy::channel(curr));

            // If there are no locally-defined channels, check the root.

            if (channels.empty())
                for (curr = find(root,       "channel"); curr;
                     curr = next(root, curr, "channel"))
                    channels.push_back(new dpy::channel(curr));

            // Start the network syncronization.

            init_server(node);
            init_client(node);
            init_listen(node);
        }

        // Determine the overlay area.

        if (app::node over = find(root, "overlay"))
        {
            int w = get_attr_d(over, "w", DEFAULT_PIXEL_WIDTH);
            int h = get_attr_d(over, "h", DEFAULT_PIXEL_HEIGHT);

            if      ((curr = app::find(over, "frustum")))
                overlay = new app::frustum(curr, w, h);
            else
                overlay = new app::frustum(0,    w, h);
        }
    }

    // If no channels or displays were configured, instance defaults.

    if (channels.empty()) channels.push_back(new dpy::channel(0));
    if (displays.empty()) displays.push_back(new dpy::normal (0));

    // If no overlay has been defined, clone the first display frustum.

    if (overlay == 0)
        overlay = new app::frustum(*(displays.front()->get_overlay()));
}

app::host::~host()
{
    if (overlay)
        delete overlay;

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        delete (*i);
    
    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        delete (*i);
    
    fini_script();
    fini_client();
    fini_server();
    fini_listen();
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    event E;

    // Kick things off with a START event.

//  process_event(E.mk_start());
    process_start(E.mk_start());  // HACK

    // Process incoming events until an exit is posted.

    while (exit_flag == false)
    {
        // Translate and dispatch SDL events.

        SDL_Event e;

        while (SDL_PollEvent(&e))
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                if (pointer_to_3D(&E, e.motion.x, e.motion.y))
                    process_event(&E);
                break;

            case SDL_MOUSEBUTTONDOWN:
                process_event(E.mk_click(0, e.button.button,
                                         SDL_GetModState(), true));
                break;

            case SDL_MOUSEBUTTONUP:
                process_event(E.mk_click(0, e.button.button,
                                         SDL_GetModState(), false));
                break;

            case SDL_KEYDOWN:
                process_event(E.mk_keybd(e.key.keysym.unicode,
                                         e.key.keysym.sym,
                                         SDL_GetModState(), true));
                break;

            case SDL_KEYUP:
                process_event(E.mk_keybd(e.key.keysym.unicode,
                                         e.key.keysym.sym,
                                         SDL_GetModState(), false));
                break;

            case SDL_JOYAXISMOTION:
                process_event(E.mk_value(e.jaxis.which,
                                         e.jaxis.axis,
                                         e.jaxis.value / 32768.0));
                break;

            case SDL_JOYBUTTONDOWN:
                process_event(E.mk_click(e.jbutton.which,
                                         e.jbutton.button, 0, true));
                break;

            case SDL_JOYBUTTONUP:
                process_event(E.mk_click(e.jbutton.which,
                                         e.jbutton.button, 0, false));
                break;

            case SDL_USEREVENT:
                process_event(E.mk_flush());
                break;

            case SDL_QUIT:
                process_event(E.mk_close());
                break;
            }

        if (exit_flag == false)
        {
            // Check for script input events.

//          poll_script();
            poll_listen();

            // Advance to the current time, or by one JIFFY when benchmarking.

            for (int tick = bench ? (tock + JIFFY) : SDL_GetTicks();
                 tick - tock >= JIFFY;
                 tock        += JIFFY)

                process_event(E.mk_timer(JIFFY));

            // Call the render handler.

            process_event(E.mk_paint());
            process_event(E.mk_frame());

            draw_flag = false;

            // Count frames and record a movie, if requested.
        
            if (movie)
            {
                count++;

                if ((count % movie) == 0)
                {
                    char buf[256];

                    sprintf(buf, "frame%05d.png", count / movie);

                    ::prog->screenshot(std::string(buf),
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

    while (exit_flag == false)
        process_event(E.recv(server_sd));
}

void app::host::loop()
{
    // Handle all events.

    if (root())
        root_loop();
    else
        node_loop();
}

//-----------------------------------------------------------------------------

void app::host::draw()
{
    // Channel and frustum vectors are passed C-style.

    int            chanc =  channels.size();
    int            frusc =  frustums.size();
    dpy::channel **chanv = &channels.front();
    app::frustum **frusv = &frustums.front();

    // Prepare all displays for rendering (cheap).

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->prep(chanc, chanv);

    // Cache the transformed frustums (cheap).

    for (app::frustum_i i = frustums.begin(); i != frustums.end(); ++i)
        (*i)->calc_view_planes(::user->get_M(),
                               ::user->get_I());

    // Do render prepass and determine view distance (possibly expensive).

    ogl::range r = ::prog->prep(frusc, frusv);

    // Cache the frustum projections (cheap).

    for (app::frustum_i i = frustums.begin(); i != frustums.end(); ++i)
    {
        (*i)->calc_projection (r.get_n(), r.get_f());
        (*i)->calc_view_points(r.get_n(), r.get_f());
    }

    // Clear the entire window.

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    // Render all displays (probably very expensive).
    
    int frusi = 0;

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        if (calibration_state)
            frusi += (*i)->test(chanc, chanv, calibration_index);
        else
            frusi += (*i)->draw(chanc, chanv, frusi, frusv);
}

void app::host::swap() const
{
    // If doing network sync, wait until the rendering has finished.

    if (server_sd != INVALID_SOCKET || !client_sd.empty())
        glFinish();

    SDL_GL_SwapBuffers();
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

// Ask each display to project the event into the virtual space.

bool app::host::pointer_to_3D(event *E, int x, int y)
{
    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        if ((*i)->pointer_to_3D(E, x, y))
            return true;

    return false;
}

void app::host::process_start(event *E)
{
    // Forward the start event to all channels and displays.

    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        (*i)->process_event(E);
    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->process_event(E);

    // Make a list of all display frustums.

    for (dpy::display_i i = displays.begin(); i != displays.end(); ++i)
        (*i)->get_frustums(frustums);

    // Start the timer.

    tock = SDL_GetTicks();
}

void app::host::process_close(event *E)
{
    post_exit();

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

    // Ensure START events are processed by the host before anyone else.

    if (E->get_type() == E_START)
    {
        process_start(E);
        sync();
        return true;
    }

    // Allow the application or the calibration to process the event.

    if (::prog->process_event(E)
        ||      process_calib(E))
        return true;

    // Handle the event locally, as needed.

    switch (E->get_type())
    {
    case E_PAINT: draw(); sync();           return true;
    case E_FRAME: swap();                   return true;
    case E_CLOSE: process_close(E); sync(); return true;
    case E_FLUSH: ::glob->fini();
                  ::glob->init(); return true;
    }

    return false;
}

//-----------------------------------------------------------------------------

int app::host::get_window_m() const
{
    return ((window_full  ? SDL_FULLSCREEN : 0) |
            (window_frame ? 0 : SDL_NOFRAME));
}

// Forward the given position and orientation to all channel objects.  This is
// usually called per-frame in response to a head motion tracker input.

void app::host::set_head(const double *p,
                         const double *q)
{
    for (dpy::channel_i i = channels.begin(); i != channels.end(); ++i)
        (*i)->set_head(p, q);
}

//-----------------------------------------------------------------------------
