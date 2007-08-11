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
#include <stdexcept>

#include <string.h>
#include <errno.h>

#include "opengl.hpp"
#include "data.hpp"
#include "view.hpp"
#include "host.hpp"
#include "prog.hpp"

#define JIFFY (1000 / 60)

#define MXML_FORALL(t, i, n) \
    for (i = mxmlFindElement((t), (t), (n), 0, 0, MXML_DESCEND_FIRST); i; \
         i = mxmlFindElement((i), (t), (n), 0, 0, MXML_NO_DESCEND))

//=============================================================================

class host_error : public std::runtime_error
{
    std::string mesg(const char *s) {
        return std::string(s) + ": " + hstrerror(h_errno);
    }

public:
    host_error(const char *s) : std::runtime_error(mesg(s)) { }
};

class sock_error : public std::runtime_error
{
    std::string mesg(const char *s) {
        return std::string(s) + ": " + strerror(errno);
    }

public:
    sock_error(const char *s) : std::runtime_error(mesg(s)) { }
};

//-----------------------------------------------------------------------------

#define E_REPLY 0
#define E_TRACK 1
#define E_STICK 2
#define E_POINT 3
#define E_CLICK 4
#define E_KEYBD 5
#define E_TIMER 6
#define E_PAINT 7
#define E_CLOSE 8

//-----------------------------------------------------------------------------

app::message::message(unsigned char type) : index(0)
{
    payload.type = type;
    payload.size = 0;
}

void app::message::put_double(double d)
{
    // Append a double to the payload data.

    union swap s;

    s.d = d;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);
    ((uint32_t *) (payload.data + payload.size))[1] = htonl(s.l[1]);

    payload.size += 2 * sizeof (uint32_t);
}

double app::message::get_double()
{
    // Return the next double in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);
    s.l[1] = ntohl(((uint32_t *) (payload.data + index))[1]);

    index += 2 * sizeof (uint32_t);

    return s.d;
}

void app::message::put_bool(bool b)
{
    payload.data[payload.size++] = (b ? 1 : 0);
}

bool app::message::get_bool()
{
    return payload.data[index++];
}

void app::message::put_int(int i)
{
    // Append an int to the payload data.

    union swap s;

    s.i = i;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);

    payload.size += sizeof (uint32_t);
}

int app::message::get_int()
{
    // Return the next int in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);

    index += sizeof (uint32_t);

    return s.i;
}

//-----------------------------------------------------------------------------

void app::message::send(SOCKET s)
{
    // Send the payload on the given socket.

    if (::send(s, &payload, payload.size + 2, 0) == -1)
        throw std::runtime_error(strerror(errno));
}

void app::message::recv(SOCKET s)
{
    // Block until receipt of the payload head and data.

    if (::recv(s, &payload, 2, 0) == -1)
        throw std::runtime_error(strerror(errno));

    if (payload.size > 0)
        if (::recv(s,  payload.data, payload.size, 0) == -1)
            throw std::runtime_error(strerror(errno));

    // Reset the unpack pointer to the beginning.

    index = 0;
}

//=============================================================================

app::tile::tile(mxml_node_t *node) : type(mono_type), mode(normal_mode)
{
    mxml_node_t *elem;
    mxml_node_t *curr;

    // Set some reasonable defaults.

    BL[0] = -0.5; BL[1] = -0.5; BL[2] = -1.0;
    BR[0] = +0.5; BR[1] = -0.5; BR[2] = -1.0;
    TL[0] = -0.5; TL[1] = +0.5; TL[2] = -1.0;

    window_rect[0] = buffer_rect[0][0] = buffer_rect[1][0] =   0;
    window_rect[1] = buffer_rect[0][1] = buffer_rect[1][1] =   0;
    window_rect[2] = buffer_rect[0][2] = buffer_rect[1][2] = 800;
    window_rect[3] = buffer_rect[0][3] = buffer_rect[1][3] = 600;

    varrier[0] = 100.00;
    varrier[1] =   0.00;
    varrier[2] =   0.00;
    varrier[3] =   0.00;
    varrier[4] =   0.75;

    // Extract the window viewport rectangle.

    if ((elem = mxmlFindElement(node, node, "viewport",
                                0, 0, MXML_DESCEND_FIRST)))
    {
        if (const char *x = mxmlElementGetAttr(elem, "x"))
            window_rect[0] = atoi(x);
        if (const char *y = mxmlElementGetAttr(elem, "y"))
            window_rect[1] = atoi(y);
        if (const char *w = mxmlElementGetAttr(elem, "w"))
            window_rect[2] = atoi(w);
        if (const char *h = mxmlElementGetAttr(elem, "h"))
            window_rect[3] = atoi(h);
    }

    // Extract the buffer viewport rectangle(s).

    MXML_FORALL(elem, curr, "eye")
    {
        int i = 0;

        if (const char *e = mxmlElementGetAttr(curr, "e"))
            i = atoi(e);

        if (0 <= i && i <= 1)
        {
            if (const char *x = mxmlElementGetAttr(curr, "x"))
                buffer_rect[i][0] = atoi(x);
            if (const char *y = mxmlElementGetAttr(curr, "y"))
                buffer_rect[i][1] = atoi(y);
            if (const char *w = mxmlElementGetAttr(curr, "w"))
                buffer_rect[i][2] = atoi(w);
            if (const char *h = mxmlElementGetAttr(curr, "h"))
                buffer_rect[i][3] = atoi(h);
        }
    }

    // Extract the screen corners.

    MXML_FORALL(elem, curr, "corner")
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
            if (const char *x = mxmlElementGetAttr(curr, "x")) v[0] = atof(x);
            if (const char *y = mxmlElementGetAttr(curr, "y")) v[1] = atof(y);
            if (const char *z = mxmlElementGetAttr(curr, "z")) v[2] = atof(z);
        }
    }

    // Extract the Varrier linescreen parameters.

    if ((elem = mxmlFindElement(node, node, "varrier",
                                0, 0, MXML_DESCEND_FIRST)))
    {
        if (const char *p = mxmlElementGetAttr(elem, "p"))
            varrier[0] = atof(p);
        if (const char *a = mxmlElementGetAttr(elem, "a"))
            varrier[1] = atof(a);
        if (const char *t = mxmlElementGetAttr(elem, "t"))
            varrier[2] = atof(t);
        if (const char *s = mxmlElementGetAttr(elem, "s"))
            varrier[3] = atof(s);
        if (const char *c = mxmlElementGetAttr(elem, "c"))
            varrier[4] = atof(c);
    }
}

//-----------------------------------------------------------------------------

void app::tile::draw()
{
    double P[3] = { 0.0, 0.0, 0.0 };

    if (type == mono_type)
    {
        glViewport(window_rect[0], window_rect[1],
                   window_rect[2], window_rect[3]);
        glScissor (window_rect[0], window_rect[1],
                   window_rect[2], window_rect[3]);

        ::view->set_P(P, BL, BR, TL);
        ::prog->draw();
    }
    if (type == varrier_type)
    {
/*
        double L[3] = { 0.0, 0.0, -0.125 };
        double R[3] = { 0.0, 0.0, +0.125 };

        ::view->set_P(L, BL, BR, TL);
        ::prog->draw();

        ::view->set_P(R, BL, BR, TL);
        ::prog->draw();
*/
    }
}

//=============================================================================

void app::host::load(std::string& file,
                     std::string& tag)
{
    if (head) mxmlDelete(head);

    head = 0;
    node = 0;

    const char *buff;

    if ((buff = (const char *) ::data->load(file)))
    {
        head = mxmlLoadString(NULL, buff, MXML_TEXT_CALLBACK);
        node = mxmlFindElement(head, head, "node", "name",
                               tag.c_str(), MXML_DESCEND);
    }

    ::data->free(file);
}

//-----------------------------------------------------------------------------

unsigned long app::host::lookup(const char *hostname)
{
    struct hostent *H;
    struct in_addr  A;

    if ((H = gethostbyname(hostname)) == 0)
        throw host_error(hostname);

    memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);

    return A.s_addr;
}

void app::host::init_server()
{
    int       on = 1;
    socklen_t onlen = sizeof (int);
        
    sockaddr_t addr;
    socklen_t  addrlen = sizeof (sockaddr_t);

    // Handle any server connection.

    const char *root = mxmlElementGetAttr(node, "root");

    if (root == 0 || strcmp(root, "true"))
    {
        const char *port = mxmlElementGetAttr(node, "port");

        addr.sin_family      = AF_INET;
        addr.sin_port        = htons (atoi(port ? port : DEFAULT_PORT));
        addr.sin_addr.s_addr = INADDR_ANY;

        // Create a socket, listen, and accept a connection.

        int sd;

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw std::runtime_error(strerror(errno));

        if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &on, onlen) < 0)
            throw std::runtime_error(strerror(errno));

        if (bind(sd, (struct sockaddr *) &addr, addrlen) < 0)
            throw std::runtime_error(strerror(errno));

        listen(sd, 5);

        if ((server = accept(sd, 0, 0)) < 0)
            throw std::runtime_error(strerror(errno));

        // The incoming socket is not needed after a connection is made.

        ::close(sd);
    }
}

void app::host::init_client()
{
    int       on = 1;
    socklen_t onlen = sizeof (int);
        
    sockaddr_t addr;
    socklen_t  addrlen = sizeof (sockaddr_t);

    // Handle any client connections.

    mxml_node_t *curr;

    MXML_FORALL(node, curr, "client")
    {
        const char *name = mxmlElementGetAttr(curr, "addr");
        const char *port = mxmlElementGetAttr(curr, "port");

        // Look up the given host name.

        addr.sin_family      = AF_INET;
        addr.sin_port        = htons (atoi(port ? port : DEFAULT_PORT));
        addr.sin_addr.s_addr = lookup(     name ? name : DEFAULT_HOST);

        // Create a socket and connect.

        int cd;

        if ((cd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            throw sock_error(name);

        if (setsockopt(cd, IPPROTO_TCP, TCP_NODELAY, &on, onlen) < 0)
            throw sock_error(name);

        if (connect(cd, (struct sockaddr *) &addr, addrlen) < 0)
            throw sock_error(name);

        client.push_back(cd);
    }
}

void app::host::fini_server()
{
    // Close the server socket.

    if (server != INVALID_SOCKET)
        ::close(server);
}

void app::host::fini_client()
{
    // Close all client sockets.

    for (unsigned int i = 0; i < client.size(); ++i)
        if (client[i] != INVALID_SOCKET)
            ::close(client[i]);
}

//-----------------------------------------------------------------------------

app::host::host(std::string& file,
                std::string& tag) :
    server(INVALID_SOCKET), mods(0), head(0), node(0)
{
    window_rect[0] =   0;
    window_rect[1] =   0;
    window_rect[2] = 800;
    window_rect[3] = 600;

    // Read host.xml and configure using tag match.

    load(file, tag);

    if (node)
    {
        // Extract host config parameters.

        if (mxml_node_t *window = mxmlFindElement(node, node, "window",
                                                  0, 0, MXML_DESCEND_FIRST))
        {
            if (const char *x = mxmlElementGetAttr(window, "x"))
                window_rect[0] = atoi(x);
            if (const char *y = mxmlElementGetAttr(window, "y"))
                window_rect[1] = atoi(y);
            if (const char *w = mxmlElementGetAttr(window, "w"))
                window_rect[2] = atoi(w);
            if (const char *h = mxmlElementGetAttr(window, "h"))
                window_rect[3] = atoi(h);
        }

        // Exctract tile config parameters.

        mxml_node_t *curr;

        MXML_FORALL(node, curr, "tile")
            tiles.push_back(tile(curr));

        // Start the network syncronization.

        init_client();
        init_server();
    }

    tock = SDL_GetTicks();
}

app::host::~host()
{
    if (node)
    {
        fini_server();
        fini_client();
    }
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    while (1)
    {
        SDL_Event e;

        // While there are available events, dispatch event handlers.

        while (SDL_PollEvent(&e))
            switch (e.type)
            {
            case SDL_MOUSEMOTION:     point(e.motion.x, e.motion.y) ;  break;
            case SDL_MOUSEBUTTONDOWN: click(e.button.button,   true);  break;
            case SDL_MOUSEBUTTONUP:   click(e.button.button,   false); break;
            case SDL_KEYDOWN:         keybd(e.key.keysym.unicode,
                                            e.key.keysym.sym, 
                                            SDL_GetModState(), true);  break;
            case SDL_KEYUP:           keybd(e.key.keysym.unicode,
                                            e.key.keysym.sym,
                                            SDL_GetModState(), false); break;
            case SDL_QUIT:            close(); return;
            }

        // Call the timer handler for each jiffy that has passed.

        while (SDL_GetTicks() - tock >= JIFFY)
        {
            tock += JIFFY;
            timer(JIFFY);
        }

        // Call the paint handler.

        paint();
    }
}

void app::host::node_loop()
{
    int    a;
    int    b;
    int    c;
    bool   d;
    double p[3];
    double v[3];

    while (1)
    {
        message M(0);

        M.recv(server);

        switch (M.type())
        {
        case E_TRACK:
            a    = M.get_int();
            p[0] = M.get_double();
            p[1] = M.get_double();
            p[2] = M.get_double();
            v[0] = M.get_double();
            v[1] = M.get_double();
            v[2] = M.get_double();
            track(a, p, v);
            break;

        case E_STICK:
            a    = M.get_int();
            p[0] = M.get_double();
            p[1] = M.get_double();
            stick(a, p);
            break;

        case E_POINT:
            a = M.get_int();
            b = M.get_int();
            point(a, b);
            break;

        case E_CLICK:
            a = M.get_int ();
            d = M.get_bool();
            click(a, d);
            break;

        case E_KEYBD:
            a = M.get_int ();
            b = M.get_int ();
            c = M.get_int ();
            d = M.get_bool();
            keybd(a, b, c, d);
            break;

        case E_TIMER:
            a = M.get_int();
            timer(a);
            break;

        case E_PAINT:
            paint();
            break;

        case E_CLOSE:
            close();
            return;
        }
    }
}

void app::host::loop()
{
    if (server == INVALID_SOCKET)
        root_loop();
    else
        node_loop();
}

//-----------------------------------------------------------------------------

void app::host::draw()
{
    std::vector<tile>::iterator i;

    glClear(GL_COLOR_BUFFER_BIT);

    if (!tiles.empty())
    {
        for (i = tiles.begin(); i != tiles.end(); ++i)
            i->draw();
    }
    else
    {
        ::prog->draw();
    }
}

//-----------------------------------------------------------------------------

void app::host::send(message& M)
{
    // Send a message to all clients.

    for (SOCKET_i i = client.begin(); i != client.end(); ++i)
        M.send(*i);
}

void app::host::recv(message& M)
{
    // Receive a message from all clients (should be E_REPLY).

    for (SOCKET_i i = client.begin(); i != client.end(); ++i)
        M.recv(*i);
}

//-----------------------------------------------------------------------------

void app::host::track(int d, const double *p, const double *v)
{
}

void app::host::stick(int d, const double *p)
{
}

void app::host::point(int x, int y)
{
    message M(E_POINT);

    M.put_int(x);
    M.put_int(y);

    send(M);

    ::prog->point(x, y);
}

void app::host::click(int b, bool d)
{
    message M(E_CLICK);

    M.put_int (b);
    M.put_bool(d);

    send(M);

    ::prog->click(b, d);
}

void app::host::keybd(int c, int k, int m, bool d)
{
    message M(E_KEYBD);

    M.put_int (c);
    M.put_int (k);
    M.put_int (m);
    M.put_bool(d);

    send(M);

    mods = m;

    ::prog->keybd(k, d, c);
}

void app::host::timer(int d)
{
    message M(E_TIMER);

    M.put_int(d);

    send(M);

    ::prog->timer(d / 1000.0);
}

void app::host::paint()
{
    message M(E_PAINT);

    send(M);
    draw( );
    recv(M);

    if (server != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server);
    }

    SDL_GL_SwapBuffers();
}

void app::host::close()
{
    message M(E_CLOSE);

    send(M);
    recv(M);

    if (server != INVALID_SOCKET)
    {
        message R(E_REPLY);
        R.send(server);
    }
}

//-----------------------------------------------------------------------------
