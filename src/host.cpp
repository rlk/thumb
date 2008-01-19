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
#include "util.hpp"
#include "perf.hpp"
#include "data.hpp"
#include "conf.hpp"
#include "glob.hpp"
#include "user.hpp"
#include "host.hpp"
#include "perf.hpp"
#include "prog.hpp"

#define JIFFY (1000 / 60)

//=============================================================================

app::view::view(mxml_node_t *node, int w, int h) : w(w), h(h), back(0)
{
    // Set some reasonable defaults.

    V[0] = P[0] = 0;
    V[1] = P[1] = 0;
    V[2] = P[2] = 0;

    color[0] = color[1] = color[2] = 0xFF;

    // If we have an XML configuration node...

    if (node)
    {
        const char *c;

        if ((c = mxmlElementGetAttr(node, "x"))) V[0] = P[0] = atof(c);
        if ((c = mxmlElementGetAttr(node, "y"))) V[1] = P[1] = atof(c);
        if ((c = mxmlElementGetAttr(node, "z"))) V[2] = P[2] = atof(c);

        if ((c = mxmlElementGetAttr(node, "r"))) color[0] = atof(c);
        if ((c = mxmlElementGetAttr(node, "g"))) color[1] = atof(c);
        if ((c = mxmlElementGetAttr(node, "b"))) color[2] = atof(c);
    }
}

app::view::~view()
{
//  if (back) ::glob->free_frame(back);
}

void app::view::set_head(const double *p,
                        const double *x,
                        const double *y,
                        const double *z)
{
    // Cache the view positions in the head's coordinate system.

    P[0] = p[0] + V[0] * x[0] + V[1] * y[0] + V[2] * z[0];
    P[1] = p[1] + V[0] * x[1] + V[1] * y[1] + V[2] * z[1];
    P[2] = p[2] + V[0] * x[2] + V[1] * y[2] + V[2] * z[2];
}

void app::view::draw(const int *rect, bool focus)
{
    double frag_d[2] = { 0, 0 };
    double frag_k[2] = { 1, 1 };

    // Set the view position.

    ::user->set_P(P);

    // If we are rendering directly on-screen...

    if (::user->get_type() == user::type_mono)
    {
        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(rect[0]);
        frag_d[1] =            -double(rect[1]);
        frag_k[0] = double(w) / double(rect[2]);
        frag_k[1] = double(h) / double(rect[3]);

        // Draw the scene.

        glViewport(rect[0], rect[1], rect[2], rect[3]);

        ::prog->draw(frag_d, frag_k);
    }
    else
    {
        // Make sure the off-screen buffer exists.

        if (back == 0)
            back = ::glob->new_frame(w, h, GL_TEXTURE_RECTANGLE_ARB,
                                           GL_RGBA8, true, false);
        if (back)
        {
            back->bind();
            {
                if (::user->get_mode() == user::mode_norm)
                {
                    // Draw the scene normally.

                    glClear(GL_COLOR_BUFFER_BIT);
                    ::prog->draw(frag_d, frag_k);
                }
                if (::user->get_mode() == user::mode_test)
                {
                    float r = color[0] * (focus ? 1.0f : 0.5f);
                    float g = color[1] * (focus ? 1.0f : 0.5f);
                    float b = color[2] * (focus ? 1.0f : 0.5f);

                    // Draw the test pattern.

                    glPushAttrib(GL_COLOR_BUFFER_BIT);
                    glClearColor(r, g, b, 1);
                    glClear(GL_COLOR_BUFFER_BIT);
                    glPopAttrib();
                }
            }
            back->free();
        }
    }
}

//=============================================================================

app::tile::tile(mxml_node_t *node) : frustum(0), varrier(0)
{
    window_rect[0] = 0;
    window_rect[1] = 0;
    window_rect[2] = DEFAULT_PIXEL_WIDTH;
    window_rect[3] = DEFAULT_PIXEL_HEIGHT;

    // If we have an XML configuration node...

    if (node)
    {
        mxml_node_t *elem;

        // Check for view and tile indices.

        tile_index = get_attr_i(node, "index", -1);
         view_index = get_attr_i(node, "view",   -1);

        // Extract the window viewport rectangle.

        if ((elem = mxmlFindElement(node, node, "viewport",
                                    0, 0, MXML_DESCEND_FIRST)))
        {
            window_rect[0] = get_attr_i(elem, "x");
            window_rect[1] = get_attr_i(elem, "y");
            window_rect[2] = get_attr_i(elem, "w");
            window_rect[3] = get_attr_i(elem, "h");
        }

        // Extract the frustum parameters.

        if ((elem = mxmlFindElement(node, node, "frustum",
                                    0, 0, MXML_DESCEND_FIRST)))
            frustum = new app::frustum(elem);

        // Extract the Varrier parameters.

        if ((elem = mxmlFindElement(node, node, "varrier",
                                    0, 0, MXML_DESCEND_FIRST)))
            varrier = new app::varrier(elem);
    }
}

app::tile::~tile()
{
    if (varrier) delete varrier;
    if (frustum) delete frustum;
}

//-----------------------------------------------------------------------------

bool app::tile::input_point(double x, double y)
{
    if (frustum && frustum->input_point(x, y))
        return true;
    else
        return false;
}

bool app::tile::input_click(int b, int m, bool d)
{
    if (frustum && frustum->input_click(b, m, d))
        return true;
    else
        return false;
}

bool app::tile::input_keybd(int k, int m, bool d)
{
     if     (varrier && varrier->input_keybd(k, m, d))
        return true;
    else if (frustum && frustum->input_keybd(k, m, d))
        return true;
    else
        return false;
}

//-----------------------------------------------------------------------------

bool app::tile::pick(double *p, double *v, int x, int y)
{
    // Determine whether the given pointer position lies within this tile.

    if (window_rect[0] <= x && x < window_rect[0] + window_rect[2] &&
        window_rect[1] <= y && y < window_rect[1] + window_rect[3])
    {
        double kx = double(x - window_rect[0]) / double(window_rect[2]);
        double ky = double(y - window_rect[1]) / double(window_rect[3]);

        // It does.  Compute the eye-space vector given by the pointer.

        p[0] = 0.0;
        p[1] = 0.0;
        p[2] = 0.0;

        v[0] = TL[0] * (1 - kx - ky) + TR[0] * kx + BL[0] * ky;
        v[1] = TL[1] * (1 - kx - ky) + TR[1] * kx + BL[1] * ky;
        v[2] = TL[2] * (1 - kx - ky) + TR[2] * kx + BL[2] * ky;

        // TODO:  Which frustum?  Which view?

        normalize(v);

        return true;
    }
    return false;
}

void app::tile::draw(std::vector<view *>& views, int current_index)
{
    const bool focus = (current_index == tile_index);

    // Apply the tile corners.

    double bl[3], br[3], tl[3], tr[3];

    get_BL(bl);
    get_BR(br);
    get_TL(tl);
    get_TR(tr);

    ::user->set_V(bl, br, tl, tr);

    // Render the view from each view.

    std::vector<view *>::iterator i;
    int e;

    for (e = 0, i = views.begin(); i != views.end(); ++i, ++e)
        if (view_index < 0 || view_index == e)
            (*i)->draw(window_rect, focus);

    // Render the onscreen exposure.

    if (const ogl::program *prog = ::user->get_prog())
    {
        double frag_d[2] = { 0, 0 };
        double frag_k[2] = { 1, 1 };

        int w = ::host->get_buffer_w();
        int h = ::host->get_buffer_h();
        int t;

        // Bind the view buffers.  Apply the Varrier transform for each.

        for (t = 0, i = views.begin(); i != views.end(); ++i, ++t)
        {
            (*i)->bind(GL_TEXTURE0 + t);
        
            glActiveTextureARB(GL_TEXTURE0 + t);
            apply_varrier((*i)->get_P());
            glActiveTextureARB(GL_TEXTURE0);
        }

        // Compute the on-screen to off-screen fragment transform.

        frag_d[0] =            -double(window_rect[0]);
        frag_d[1] =            -double(window_rect[1]);
        frag_k[0] = double(w) / double(window_rect[2]);
        frag_k[1] = double(h) / double(window_rect[3]);

        // Draw the tile region.

        glViewport(window_rect[0], window_rect[1],
                   window_rect[2], window_rect[3]);

        glMatrixMode(GL_PROJECTION);
        {
            // HACK: breaks varrier

            glLoadIdentity();
            glOrtho(0, window_rect[2],
                    0, window_rect[3], -1, +1);

//          glOrtho(-W / 2, +W / 2, -H / 2, +H / 2, -1, +1);
        }
        glMatrixMode(GL_MODELVIEW);
        {
            glLoadIdentity();
        }

        prog->bind();
        {
            prog->uniform("L_map", 0);
            prog->uniform("R_map", 1);

            prog->uniform("cycle", varrier_cycle);
            prog->uniform("offset", -W / (3 * window_rect[2]), 0,
                                     W / (3 * window_rect[2]));

            prog->uniform("frag_d", frag_d[0], frag_d[1]);
            prog->uniform("frag_k", frag_k[0], frag_k[1]);

            if (reg) reg->draw();
        }
        prog->free();

        if (reg && focus && ::user->get_mode() == app::user::mode_test)
            reg->wire();

        // Free the view buffers...

        for (t = GL_TEXTURE0, i = views.begin(); i != views.end(); ++i, ++t)
            (*i)->free(t);
    }
}

//=============================================================================

static const char *save_cb(mxml_node_t *node, int where)
{
    std::string name(node->value.element.name);

    switch (where)
    {
    case MXML_WS_AFTER_OPEN:  return "\n";
    case MXML_WS_AFTER_CLOSE: return "\n";
        
    case MXML_WS_BEFORE_OPEN:
        if      (name == "view")      return "  ";
        else if (name == "gui")      return "  ";
        else if (name == "node")     return "  ";
        else if (name == "buffer")   return "  ";

        else if (name == "tile")     return "    ";
        else if (name == "window")   return "    ";

        else if (name == "viewport") return "      ";
        else if (name == "varrier")  return "      ";
        else if (name == "region")   return "      ";
        else if (name == "corner")   return "        ";
        else if (name == "rotate")   return "        ";
        break;

    case MXML_WS_BEFORE_CLOSE:
        if      (name == "gui")      return "  ";
        else if (name == "node")     return "  ";
        else if (name == "tile")     return "    ";
        else if (name == "region")   return "      ";
        break;
    }

    return NULL;
}

void app::host::save()
{
    if (dirty)
    {
        char *buff;

        if ((buff = mxmlSaveAllocString(head, save_cb)))
        {
            ::data->save(file, buff);
            free(buff);
        }

        dirty = false;
    }
}

void app::host::load(std::string& tag)
{
    if (head) mxmlDelete(head);

    head = 0;
    node = 0;

    const char *buff;

    try
    {
        // Load the named host config file.

        if ((buff = (const char *) ::data->load(file)))
        {
            head = mxmlLoadString(NULL, buff, MXML_TEXT_CALLBACK);
            node = mxmlFindElement(head, head, "node", "name",
                                   tag.c_str(), MXML_DESCEND);
        }
        ::data->free(file);
    }
    catch (app::find_error& e)
    {
        // If the host config file is missing, rely upon defaults.
    }
}

//-----------------------------------------------------------------------------

static unsigned long lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)) == 0)
        throw host_error(hostname);

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

void app::host::fork_client(const char *addr, const char *name)
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

void app::host::init_listen()
{
    const char *port = mxmlElementGetAttr(node, "port");

    // If we have a port assignment then we must listen on it.

    if (port)
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        address.sin_family      = AF_INET;
        address.sin_port        = htons(atoi(port));
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
                    throw sock_error("select");
            }
            else
            {
                // Accept the connection.

                if (int sd = accept(listen_sd, 0, 0))
                {
                    if (sd < 0)
                        throw sock_error("accept");
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

void app::host::init_server()
{
    // If we have a server assignment then we must connect to it.

    if (mxml_node_t *server = mxmlFindElement(node, node, "server",
                                              0, 0, MXML_DESCEND_FIRST))
    {
        socklen_t  addresslen = sizeof (sockaddr_t);
        sockaddr_t address;

        const char *addr = mxmlElementGetAttr(server, "addr");
        const char *port = mxmlElementGetAttr(server, "port");

        // Look up the given host name.

        address.sin_family      = AF_INET;
        address.sin_port        = htons (atoi(port ? port : DEFAULT_PORT));
        address.sin_addr.s_addr = lookup(     addr ? addr : DEFAULT_HOST);

        // Create a socket and connect.

        if ((server_sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw sock_error(addr);

        while (connect(server_sd, (struct sockaddr *) &address, addresslen) <0)
            if (errno == ECONNREFUSED)
            {
                std::cerr << "Waiting for " << addr << std::endl;
                usleep(250000);
            }
            else throw sock_error(addr);

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

void app::host::init_client()
{
    mxml_node_t *curr;

    // Launch all client processes.

    MXML_FORALL(node, curr, "client")
    {
        const char *name = mxmlElementGetAttr(curr, "name");
        const char *addr = mxmlElementGetAttr(curr, "addr");

        fork_client(addr, name);
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

// TODO: move this to a separate module?

static void overlay(mxml_node_t *node, double *M, double *I, int w, int h)
{
    double a = double(DEFAULT_PIXEL_WIDTH) / double(DEFAULT_PIXEL_HEIGHT);
    const char *c;

    double BL[3];
    double BR[3];
    double TL[3];

    BL[0] = -0.5 * a; BL[1] = -0.5; BL[2] = -1.0;
    BR[0] = +0.5 * a; BR[1] = -0.5; BR[2] = -1.0;
    TL[0] = -0.5 * a; TL[1] = +0.5; TL[2] = -1.0;

    // Extract overlay corners from the given node.

    mxml_node_t *curr;

    MXML_FORALL(node, curr, "corner")
    {
        double *v = 0;

        if (const char *name = mxmlElementGetAttr(curr, "name"))
        {
            if      (strcmp(name, "BL") == 0) v = BL;
            else if (strcmp(name, "BR") == 0) v = BR;
            else if (strcmp(name, "TL") == 0) v = TL;
        }
        if (v)
        {
            if ((c = mxmlElementGetAttr(curr, "x"))) v[0] = atof(c);
            if ((c = mxmlElementGetAttr(curr, "y"))) v[1] = atof(c);
            if ((c = mxmlElementGetAttr(curr, "z"))) v[2] = atof(c);
        }
    }

    // Compose a transform taking overlay coordinates to eye coordinates.

    double G[16];
    double R[3];
    double U[3];
    double B[3];

    R[0] = BR[0] - BL[0];
    R[1] = BR[1] - BL[1];
    R[2] = BR[2] - BL[2];

    U[0] = TL[0] - BL[0];
    U[1] = TL[1] - BL[1];
    U[2] = TL[2] - BL[2];

    crossprod(B, R, U);
    set_basis(G, R, U, B);

    load_idt(M);

    Rmul_xlt_mat(M, BL[0], BL[1], BL[2]);
    mult_mat_mat(M, M, G);
    Rmul_scl_inv(M, w, h, 1);

    if (I) load_inv(I, M);
}

app::host::host(std::string file,
                std::string tag) :
    server_sd(INVALID_SOCKET),
    listen_sd(INVALID_SOCKET),
    mods(0),
    logo_text(0),
    file(file),
    head(0),
    node(0),
    dirty(false),
    current_index(0)
{
    const char *c;

    // Set some reasonable defaults.

    window_rect[0] = 0;
    window_rect[1] = 0;
    window_rect[2] = DEFAULT_PIXEL_WIDTH;
    window_rect[3] = DEFAULT_PIXEL_HEIGHT;

    buffer_w = DEFAULT_PIXEL_WIDTH;
    buffer_h = DEFAULT_PIXEL_HEIGHT;

    gui_w = DEFAULT_PIXEL_WIDTH;
    gui_h = DEFAULT_PIXEL_HEIGHT;

    load_idt(gui_M);
    load_idt(gui_I);
    load_idt(tag_M);

    // Read host.xml and configure using tag match.

    load(tag);

    if (head)
    {
        mxml_node_t *curr;

        // Extract GUI overlay configuration.

        if (mxml_node_t *gui = mxmlFindElement(head, head, "gui",
                                               0, 0, MXML_DESCEND))
        {
            if (const char *w = mxmlElementGetAttr(gui, "w")) gui_w = atoi(w);
            if (const char *h = mxmlElementGetAttr(gui, "h")) gui_h = atoi(h);

            overlay(gui, gui_M, gui_I, gui_w, gui_h);
        }

        // Extract tag overlay configuration.

        if (mxml_node_t *tag = mxmlFindElement(head, head, "tag",
                                               0, 0, MXML_DESCEND))
        {
            if (const char *name = mxmlElementGetAttr(tag, "name"))
                logo_name = std::string(name);

            overlay(tag, tag_M, 0, 1, 1);
        }

        // Extract global off-screen buffer configuration.

        if (mxml_node_t *buffer = mxmlFindElement(head, head, "buffer",
                                                  0, 0, MXML_DESCEND))
        {
            if ((c = mxmlElementGetAttr(buffer, "w"))) buffer_w = atoi(c);
            if ((c = mxmlElementGetAttr(buffer, "h"))) buffer_h = atoi(c);
        }

        // Extract view config parameters

        MXML_FORALL(head, curr, "view")
            views.push_back(new view(curr, buffer_w, buffer_h));
    }

    if (node)
    {
        mxml_node_t *curr;

        // Extract the window parameters.

        if (mxml_node_t *window = mxmlFindElement(node, node, "window",
                                                  0, 0, MXML_DESCEND_FIRST))
        {
            if ((c = mxmlElementGetAttr(window, "x"))) window_rect[0] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "y"))) window_rect[1] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "w"))) window_rect[2] =atoi(c);
            if ((c = mxmlElementGetAttr(window, "h"))) window_rect[3] =atoi(c);
        }

        // Extract local off-screen buffer configuration.

        if (mxml_node_t *buffer = mxmlFindElement(node, node, "buffer",
                                                  0, 0, MXML_DESCEND_FIRST))
        {
            if ((c = mxmlElementGetAttr(buffer, "w"))) buffer_w = atoi(c);
            if ((c = mxmlElementGetAttr(buffer, "h"))) buffer_h = atoi(c);
        }

        // Extract tile config parameters.

        MXML_FORALL(node, curr, "tile")
            tiles.push_back(new tile(curr));

        // Start the network syncronization.

        init_listen();
        init_server();
        init_client();
    }

    // If no views or tiles were defined, instance defaults.

    if (views.empty()) views.push_back(new view(0, buffer_w, buffer_h));
    if (tiles.empty()) tiles.push_back(new tile(0));

    // Start the timer.

    tock = SDL_GetTicks();
}

app::host::~host()
{
    std::vector<view *>::iterator i;

    for (i = views.begin(); i != views.end(); ++i)
        delete (*i);
    
    if (node)
    {
        fini_client();
        fini_server();
        fini_listen();
    }
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
                      e.jbutton.button, true);
                break;

            case SDL_JOYBUTTONUP:
                click(e.jbutton.which,
                      e.jbutton.button, false);
                break;

            case SDL_QUIT:
                save();
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

            if (tracker_values(0, a)) value(1, 0, v);
            if (tracker_values(1, a)) value(1, 1, v);

            if (tracker_sensor(0, p, q)) point(1, p, q);
            if (tracker_sensor(1, p, q)) point(2, p, q);
        }

        // Call the timer handler for each jiffy that has passed.

        while (SDL_GetTicks() - tock >= JIFFY)
        {
            tock += JIFFY;
            timer(JIFFY);
        }

        // Call the paint handler.

        paint();
        front();
    }
}

void app::host::node_loop()
{
    int    i;
    int    a;
    int    b;
    int    c;
    int    m;
    bool   d;
    double p[3];
    double q[3];

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
        case E_FLEEP:
        {
            fleep();
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
    // Determine the frustum union and preprocess the app.

    double F[32];

    get_frustum(F);
    ::prog->prep(F, 4);

    // Render all tiles.
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<tile *>::iterator i;

    for (i = tiles.begin(); i != tiles.end(); ++i)
        (*i)->draw(views, current_index);

    // If doing network sync, wait until the rendering has finished.

    if (server_sd != INVALID_SOCKET || !client_sd.empty())
        glFinish();
}

int app::host::get_window_m() const
{
    return (server_sd == INVALID_SOCKET) ? 0 : SDL_NOFRAME;
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

    if (::user->get_mode() == app::user::mode_test)
    {
        if (tile_input_point(i, p, q))
        {
            dirty = true;
            return;
        }
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

    if (::user->get_mode() == app::user::mode_test)
    {
        if (tile_input_click(i, b, m, d))
        {
            dirty = true;
            return;
        }
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

    if (d && ::host->modifiers() & KMOD_CTRL)
    {
        // Toggling calibration mode?

        if (k == SDLK_TAB)
        {
            if (::user->get_mode() == app::user::mode_norm)
                ::user->set_mode(app::user::mode_test);
            else
                ::user->set_mode(app::user::mode_norm);
            return;
        }

        // Selecting calibration tile?

        else if (k == SDLK_INSERT)
        {
            current_index++;
            return;
        }
        else if (k == SDLK_DELETE)
        {
            current_index--;
            return;
        }

        // Calibrating a tile?

        else if (::user->get_mode() == app::user::mode_test)
        {
            if (tile_input_keybd(c, k, m, d))
            {
                dirty = true;
                return;
            }
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
    ::perf->step();
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
                         const double *x,
                         const double *y,
                         const double *z)
{
    std::vector<view *>::iterator i;

    for (i = views.begin(); i != views.end(); ++i)
        (*i)->set_head(p, x, y, z);
}

//-----------------------------------------------------------------------------

void app::host::gui_pick(int& x, int& y, const double *p,
                                         const double *v) const
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

//-----------------------------------------------------------------------------

bool app::host::tile_input_point(int i, const double *p, const double *q)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(current_index) && (*t)->input_point(i, p, v))
            return true;

    return false;
}

bool app::host::tile_input_click(int i, int b, int m, bool d)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(current_index) && (*t)->input_click(i, b, m, d))
            return true;

    return false;
}

bool app::host::tile_input_keybd(int c, int k, int m, bool d)
{
    for (tile_i t = tiles.begin(); t != tiles.end(); ++t)
        if ((*t)->is_index(current_index) && (*t)->input_keybd(c, k, m, d))
            return true;

    return false;
}

//-----------------------------------------------------------------------------
