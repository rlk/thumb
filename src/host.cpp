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

#include "tracker.hpp"
#include "opengl.hpp"
#include "matrix.hpp"
#include "data.hpp"
#include "conf.hpp"
#include "glob.hpp"
#include "user.hpp"
#include "host.hpp"
#include "perf.hpp"
#include "prog.hpp"

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

static void nodelay(int sd)
{
    socklen_t len = sizeof (int);
    int       val = 1;
        
    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val, len) < 0)
        throw std::runtime_error(strerror(errno));
}

//-----------------------------------------------------------------------------

void app::host::fork_client(const char *name, const char *addr)
{
    const char *args[4];
    char line[256];

    if ((fork() == 0))
    {
        std::string dir = ::conf->get_s("exec_dir");

        sprintf(line, "cd %s; ./thumb %s", dir.c_str(), name);

        // Allocate and build the client's ssh command line.

        args[0] = "ssh";
        args[1] = addr;
        args[2] = line;
        args[3] = NULL;

        execvp("ssh", (char * const *) args);

        exit(0);
    }
}

//-----------------------------------------------------------------------------

void app::host::init_listen(app::node node)
{
    int port = get_attr_d(node, "port");

    // If we have a port assignment then we must listen on it.

    if (port)
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        address.sin_family      = AF_INET;
        address.sin_port        = htons(port);
        address.sin_addr.s_addr = INADDR_ANY;

        // Create a socket, set no-delay, bind the port, and listen.

        if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw std::runtime_error(strerror(errno));
        
        nodelay(listen_sd);

        while (bind(listen_sd, (struct sockaddr *) &address, addresslen) < 0)
            if (errno == EINVAL)
            {
                std::cerr << "Waiting for port expiration" << std::endl;
                usleep(250000);
            }
            else throw std::runtime_error(strerror(errno));

        listen(listen_sd, 16);
    }
}

void app::host::poll_listen()
{
    // NOTE: This must not occur between a host::send/host::recv pair.

    if (listen_sd != INVALID_SOCKET)
    {
        struct timeval tv = { 0, 0 };

        fd_set sd;
        
        FD_ZERO(&sd);
        FD_SET(listen_sd, &sd);

        // Check for an incomming client connection.

        if (int n = select(listen_sd + 1, &sd, NULL, NULL, &tv))
        {
            if (n < 0)
            {
                if (errno != EINTR)
                    throw app::sock_error("select");
            }
            else
            {
                // Accept the connection.

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
}

void app::host::fini_listen()
{
    if (listen_sd != INVALID_SOCKET)
        ::close(listen_sd);

    listen_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void app::host::init_server(app::node node)
{
    // If we have a server assignment then we must connect to it.

    if (app::node server = find(node, "server"))
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        const char *addr = get_attr_s(server, "addr", DEFAULT_HOST);
        int         port = get_attr_d(server, "port", DEFAULT_PORT);

        // Look up the given host name.

        address.sin_family      = AF_INET;
        address.sin_port        = htons (port);
        address.sin_addr.s_addr = lookup(addr);

        // Create a socket and connect.

        if ((server_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw app::sock_error(addr);

        while (connect(server_sd, (struct sockaddr *) &address, addresslen) <0)
            if (errno == ECONNREFUSED)
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
        fork_client(get_attr_s(curr, "name"),
                    get_attr_s(curr, "addr"));
    }
}

void app::host::fini_client()
{
    for (unsigned int i = 0; i < client_sd.size(); ++i)
    {
        char c;

        // Wait for EOF (orderly shutdown by the remote).

        while (::recv(client_sd[i], &c, 1, 0) > 0)
            ;

        // Close the socket.

        ::close(client_sd[i]);
    }
}

//-----------------------------------------------------------------------------

app::host::host(std::string filename, std::string tag) :
    server_sd(INVALID_SOCKET),
    listen_sd(INVALID_SOCKET),
    bench(::conf->get_i("bench")),
    calibrate_state(false),
    calibrate_index(0),
    file(filename.c_str())
{
    // Set some reasonable defaults.

    window[0] = 0;
    window[1] = 0;
    window[2] = DEFAULT_PIXEL_WIDTH;
    window[3] = DEFAULT_PIXEL_HEIGHT;

    buffer[0] = DEFAULT_PIXEL_WIDTH;
    buffer[1] = DEFAULT_PIXEL_HEIGHT;

    // Read host.xml and configure using tag match.

    app::node root;
    app::node node;
    app::node curr;

    if ((root = find(file.get_head(), "host")))
    {
        // Extract global off-screen buffer configuration.

        if (app::node buf = find(root, "buffer"))
        {
            buffer[0] = get_attr_d(buf, "w", DEFAULT_PIXEL_WIDTH);
            buffer[1] = get_attr_d(buf, "h", DEFAULT_PIXEL_HEIGHT);
        }

        // Locate the configuration for this node.

        if ((node = find(root, "node", "name", tag.c_str())))
        {
            // Extract the on-screen window configuration.

            if (app::node win = find(node, "window"))
            {
                window[0] = get_attr_d(win, "x", 0);
                window[1] = get_attr_d(win, "y", 0);
                window[2] = get_attr_d(win, "w", DEFAULT_PIXEL_WIDTH);
                window[3] = get_attr_d(win, "h", DEFAULT_PIXEL_HEIGHT);
            }

            // Extract local off-screen buffer configuration.

            if (app::node buf = find(node, "buffer"))
            {
                buffer[0] = get_attr_d(buf, "w", DEFAULT_PIXEL_WIDTH);
                buffer[1] = get_attr_d(buf, "h", DEFAULT_PIXEL_HEIGHT);
            }

            // Create a tile object for each configured tile.

            for (curr = find(node,       "tile"); curr;
                 curr = next(node, curr, "tile"))
                tiles.push_back(new tile(curr));

            // Start the network syncronization.

            init_listen(node);
            init_server(node);
            init_client(node);
        }
    }

    // Create a view object for each configured view.

    for (curr = find(root,       "view"); curr;
         curr = next(root, curr, "view"))
        views.push_back(new view(curr, buffer));

    // If no views or tiles were configured, instance defaults.

    if (views.empty()) views.push_back(new view(0, buffer));
    if (tiles.empty()) tiles.push_back(new tile(0));

    // Start the timer.

    tock = SDL_GetTicks();
}

app::host::~host()
{
    std::vector<view *>::iterator i;

    for (i = views.begin(); i != views.end(); ++i)
        delete (*i);
    
    fini_client();
    fini_server();
    fini_listen();
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    std::vector<tile *>::iterator i;

    double p[3];
    double q[4];

    // TODO: decide on a device numbering for mouse/trackd/joystick

    while (1)
    {
        SDL_Event e;

        // While there are available SDL events, dispatch event handlers.

        while (SDL_PollEvent(&e))
            switch (e.type)
            {
            case SDL_MOUSEMOTION:
                for (i = tiles.begin(); i != tiles.end(); ++i)
                    if ((*i)->pick(p, q, e.motion.x, e.motion.y))
                        point(0, p, q);
                break;

            case SDL_MOUSEBUTTONDOWN:
                click(0, e.button.button, SDL_GetModState(), true);
                break;

            case SDL_MOUSEBUTTONUP:
                click(0, e.button.button, SDL_GetModState(), false);
                break;

            case SDL_KEYDOWN:
                keybd(e.key.keysym.unicode,
                      e.key.keysym.sym,
                      e.key.keysym.mod, true);
                break;

            case SDL_KEYUP:
                keybd(e.key.keysym.unicode,
                      e.key.keysym.sym,
                      e.key.keysym.mod, false);
                break;

            case SDL_JOYAXISMOTION:
                value(e.jaxis.which,
                      e.jaxis.axis,
                      e.jaxis.value / 32768.0);
                break;

            case SDL_JOYBUTTONDOWN:
                click(e.jbutton.which,
                      e.jbutton.button, 0, true);
                break;

            case SDL_JOYBUTTONUP:
                click(e.jbutton.which,
                      e.jbutton.button, 0, false);
                break;

            case SDL_QUIT:
                close();
                return;
            }

        // Dispatch tracker events.

        if (tracker_status())
        {
            double p[3];
            double q[3];
            double v;
            bool   b;

            if (tracker_button(0, b)) click(1, 0, 0, b);
            if (tracker_button(1, b)) click(1, 1, 0, b);
            if (tracker_button(2, b)) click(1, 2, 0, b);

            if (tracker_values(0, v)) value(1, 0, v);
            if (tracker_values(1, v)) value(1, 1, v);

            if (tracker_sensor(0, p, q)) point(1, p, q);
            if (tracker_sensor(1, p, q)) point(2, p, q);
        }

        // Call the timer handler for each jiffy that has passed.

        if (bench)
        {
            timer(JIFFY);
        }
        else
        {
            int tick = SDL_GetTicks();

            while (tick - tock >= JIFFY)
            {
                tock += JIFFY;
                timer(JIFFY);
            }
        }

        // Call the paint handler.

        paint();
        front();
    }
}

void app::host::node_loop()
{
    while (1)
    {
        message M(0);

        M.recv(server_sd);

        switch (M.type())
        {
        case E_POINT:
        {
            double p[3];
            double q[4];

            int i = M.get_byte();
            p[0]  = M.get_real();
            p[1]  = M.get_real();
            p[2]  = M.get_real();
            q[0]  = M.get_real();
            q[1]  = M.get_real();
            q[2]  = M.get_real();
            q[3]  = M.get_real();
            point(i, p, q);
            break;
        }
        case E_CLICK:
        {
            int  i = M.get_byte();
            int  b = M.get_byte();
            int  m = M.get_byte();
            bool d = M.get_bool();
            click(i, b, m, d);
            break;
        }
        case E_KEYBD:
        {
            int  c = M.get_word();
            int  k = M.get_word();
            int  m = M.get_word();
            bool d = M.get_bool();
            keybd(c, k, m, d);
            break;
        }
        case E_VALUE:
        {
            int    i = M.get_byte();
            int    a = M.get_byte();
            double p = M.get_real();
            value(i, a, p);
            break;
        }
        case E_TIMER:
        {
            timer(M.get_word());
            break;
        }
        case E_PAINT:
        {
            paint();
            break;
        }
        case E_FRONT:
        {
            front();
            break;
        }
        case E_CLOSE:
        {
            close();
            return;
        }
        default:
            break;
        }
    }
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
//  if (!calibrate_state)
    {
        std::vector<frustum *> frusta;

        // Acculumate a list of frusta and preprocess the app.

        for (tile_i i = tiles.begin(); i != tiles.end(); ++i)
            (*i)->prep(views, frusta);

        ::prog->prep(frusta);
    }

    // Render all tiles.
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int c = 0;

    for (tile_i i = tiles.begin(); i != tiles.end(); ++i)
        (*i)->draw(views, c, calibrate_state, calibrate_index);

    // If doing network sync, wait until the rendering has finished.

    if (server_sd != INVALID_SOCKET || !client_sd.empty())
        glFinish();
}

int app::host::get_window_m() const
{
    return root() ? 0 : SDL_NOFRAME;
}

//-----------------------------------------------------------------------------

void app::host::send(message& M)
{
    // Send a message to all clients.

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        M.send(*i);
}

void app::host::recv(message& M)
{
    // Receive a message from all clients (should be E_REPLY).

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
        M.recv(*i);
}

//-----------------------------------------------------------------------------

void app::host::point(int i, const double *p, const double *q)
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_POINT);

        M.put_byte(i);
        M.put_real(p[0]);
        M.put_real(p[1]);
        M.put_real(p[2]);
        M.put_real(q[0]);
        M.put_real(q[1]);
        M.put_real(q[2]);
        M.put_real(q[3]);

        send(M);
    }

    // Calibrating a tile?

    if (calibrate_state)
    {
        if (tile_input_point(i, p, q))
            return;
    }

    // Let the application have the click event.

    else ::prog->point(i, p, q);
}

void app::host::click(int i, int b, int m, bool d)
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_CLICK);

        M.put_byte(i);
        M.put_byte(b);
        M.put_byte(m);
        M.put_bool(d);

        send(M);
    }

    // Calibrating a tile?

    if (calibrate_state)
    {
        if (tile_input_click(i, b, m, d))
            return;
    }

    // Let the application have the click event.

    else ::prog->click(i, b, m, d);
}

void app::host::keybd(int c, int k, int m, bool d)
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_KEYBD);

        M.put_word(c);
        M.put_word(k);
        M.put_word(m);
        M.put_bool(d);

        send(M);
    }

    if (d && (m & KMOD_CTRL))
    {
        // Toggling calibration mode?

        if (k == SDLK_TAB)
        {
            calibrate_state = !calibrate_state;
            printf("state %d\n", calibrate_state);
            return;
        }

        // Selecting calibration tile?

        else if (k == SDLK_SPACE)
        {
            calibrate_index++;
            printf("index %d\n", calibrate_index);
            return;
        }
        else if (k == SDLK_BACKSPACE)
        {
            calibrate_index--;
            printf("index %d\n", calibrate_index);
            return;
        }

        // Calibrating a tile?

        else if (calibrate_state)
        {
            tile_input_keybd(c, k, m, d);
            return;
        }
    }

    // If none of the above, let the application have the keybd event.

    ::prog->keybd(c, k, m, d);
}

void app::host::value(int i, int a, double v)
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_VALUE);

        M.put_byte(i);
        M.put_byte(a);
        M.put_real(v);

        send(M);
    }

    // Let the application handle it.

    ::prog->value(i, a, v);
}

void app::host::timer(int t)
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_TIMER);

        M.put_word(t);

        send(M);
    }

    // Let the application handle it.

    ::prog->timer(t);

    // Check for connecting clients.

    poll_listen();
}

void app::host::paint()
{
    // Forward the event to all clients, draw, and wait for sync.

    if (!client_sd.empty())
    {
        message M(E_PAINT);

        send(M);
        draw( );
        recv(M);
    }
    else draw();

    // Send sync to the server.

    if (server_sd != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server_sd);
    }
}

void app::host::front()
{
    // Forward the event to all clients.

    if (!client_sd.empty())
    {
        message M(E_FRONT);

        send(M);
    }

    // Swap the back buffer to the front.

    SDL_GL_SwapBuffers();
    ::perf->step(bench);
}

void app::host::close()
{
    // Forward the event to all clients.  Wait for sync.

    if (!client_sd.empty())
    {
        message M(E_CLOSE);

        send(M);
        recv(M);
    }

    // Sind sync to the server.

    if (server_sd != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server_sd);
    }
}

//-----------------------------------------------------------------------------

void app::host::set_head(const double *p,
                         const double *q)
{
    std::vector<view *>::iterator i;

    for (i = views.begin(); i != views.end(); ++i)
        (*i)->set_head(p, q);
}

//-----------------------------------------------------------------------------
#ifdef SNIP
void app::host::gui_pick(int& x, int& y, const double *p,
                                         const double *q) const
{
    // Compute the point (x, y) at which the vector V from P hits the GUI.

    double q[3];
    double w[3];

    // TODO: convert this to take a quaternion

    mult_mat_vec3(q, gui_I, p);
    mult_xps_vec3(w, gui_I, v);

    normalize(w);

    x = int(nearestint(q[0] - q[2] * w[0] / w[2]));
    y = int(nearestint(q[1] - q[2] * w[1] / w[2]));
}

void app::host::gui_size(int& w, int& h) const
{
    // Return the configured resolution of the GUI.

    w = gui_w;
    h = gui_h;
}

void app::host::gui_view() const
{
    // Apply the transform taking GUI coordinates to eye coordinates.

    glMultMatrixd(gui_M);
}
#endif
//-----------------------------------------------------------------------------

bool app::host::tile_input_point(int i, const double *p, const double *q)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(calibrate_index) && (*t)->input_point(i, p, q))
            return true;

    return false;
}

bool app::host::tile_input_click(int i, int b, int m, bool d)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(calibrate_index) && (*t)->input_click(i, b, m, d))
            return true;

    return false;
}

bool app::host::tile_input_keybd(int c, int k, int m, bool d)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(calibrate_index) && (*t)->input_keybd(c, k, m, d))
            return true;

    return false;
}

//-----------------------------------------------------------------------------
