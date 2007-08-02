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

#include "data.hpp"
#include "host.hpp"
#include "prog.hpp"

#define JIFFY (1000 / 60)

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

void app::host::load(std::string name)
{
    if (head) mxmlDelete(head);

    head = 0;
    node = 0;

    const char *buff;

    if ((buff = (const char *) ::data->load(DEFAULT_HOST_FILE)))
    {
        head = mxmlLoadString(NULL, buff, MXML_TEXT_CALLBACK);
        node = mxmlFindElement(head, head, "node", "name",
                               name.c_str(), MXML_DESCEND);
    }

    ::data->free(DEFAULT_HOST_FILE);
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

    const char  *str = "client";
    mxml_node_t *curr;

    for (curr = mxmlFindElement(node, node, str, 0, 0, MXML_DESCEND_FIRST);
         curr;
         curr = mxmlFindElement(curr, node, str, 0, 0, MXML_NO_DESCEND))
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

app::host::host(std::string tag) : server(INVALID_SOCKET), head(0), node(0)
{
    // Read host.xml and configure using tag match.

    load(tag);

    if (node)
    {
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
            case SDL_MOUSEMOTION:     point(e.motion.x, e.motion.y);  break;
            case SDL_MOUSEBUTTONDOWN: click(e.button.button,  true);  break;
            case SDL_MOUSEBUTTONUP:   click(e.button.button,  false); break;
            case SDL_KEYDOWN:         keybd(e.key.keysym.sym, true);  break;
            case SDL_KEYUP:           keybd(e.key.keysym.sym, false); break;
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
    bool   c;
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
            c = M.get_bool();
            click(a, c);
            break;

        case E_KEYBD:
            a = M.get_int ();
            c = M.get_bool();
            keybd(a, c);
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

void app::host::keybd(int k, bool d)
{
    message M(E_KEYBD);

    M.put_int (k);
    M.put_bool(d);

    send(M);

    ::prog->keybd(k, d, k);
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
    ::prog->draw();
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
