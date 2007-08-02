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

#include <iostream>
#include <stdexcept>

#include <string.h>
#include <errno.h>

#include "data.hpp"
#include "host.hpp"

//=============================================================================
// Simple TCP message handler

class message
{
    union swap { double d; int i; uint32_t l[2]; };

    struct { unsigned char type;
             unsigned char size;
             unsigned char data[256]; } payload;

    int index;

public:

    message(unsigned char);

    // Data marshalling

    void   put_double(double);
    void   put_int   (int);

    double get_double();
    int    get_int   ();

    // Network IO

    void send(SOCKET);
    void recv(SOCKET);
};

//-----------------------------------------------------------------------------

message::message(unsigned char type) : index(0)
{
    payload.type = type;
    payload.size = 0;
}

void message::put_double(double d)
{
    // Append a double to the payload data.

    union swap s;

    s.d = d;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);
    ((uint32_t *) (payload.data + payload.size))[1] = htonl(s.l[1]);

    payload.size += 2 * sizeof (uint32_t);
}

double message::get_double()
{
    // Return the next double in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);
    s.l[1] = ntohl(((uint32_t *) (payload.data + index))[1]);

    index += 2 * sizeof (uint32_t);

    return s.d;
}

void message::put_int(int i)
{
    // Append an int to the payload data.

    union swap s;

    s.i = i;

    ((uint32_t *) (payload.data + payload.size))[0] = htonl(s.l[0]);

    payload.size += sizeof (uint32_t);
}

int message::get_int()
{
    // Return the next int in the payload data.

    union swap s;

    s.l[0] = ntohl(((uint32_t *) (payload.data + index))[0]);

    index += sizeof (uint32_t);

    return s.i;
}

//-----------------------------------------------------------------------------

void message::send(SOCKET s)
{
    // Send the payload on the given socket.

    if (::send(s, &payload, payload.size + 2, 0) == -1)
        throw std::runtime_error(strerror(errno));
}

void message::recv(SOCKET s)
{
    // Block until receipt of the payload head and data.

    if (::recv(s, &payload, 2, 0) == -1)
        throw std::runtime_error(strerror(errno));

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
        throw name_error(hostname);

    memcpy(&A.s_addr, H->h_addr_list[0], H->h_length);

    return A.s_addr;
}

app::host::host(std::string tag) : server(INVALID_SOCKET), head(0), node(0)
{
    // Read host.xml and configure using tag match.

    load(tag);

    if (node)
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
                throw std::runtime_error(strerror(errno));

            if (setsockopt(cd, IPPROTO_TCP, TCP_NODELAY, &on, onlen) < 0)
                throw std::runtime_error(strerror(errno));

            if (connect(cd, (struct sockaddr *) &addr, addrlen) < 0)
                throw std::runtime_error(strerror(errno));

            client.push_back(cd);
        }

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
}

app::host::~host()
{
    // Close the server socket.

    if (server != INVALID_SOCKET)
        ::close(server);

    // Close all client sockets.

    for (unsigned int i = 0; i < client.size(); ++i)
        if (client[i] != INVALID_SOCKET)
            ::close(client[i]);
}

//-----------------------------------------------------------------------------

void app::host::root_loop()
{
    /* While SDL event */
    /*     call event handler */
}

void app::host::node_loop()
{
    /* While server incoming event */
    /*     call event handler */
}

void app::host::loop()
{
    if (0 /* no server */)
        root_loop();
    else
        node_loop();
}

//-----------------------------------------------------------------------------

void app::host::point(int x, int y)
{
    if (0/* has clients */)
        ; //send_point(x, y);

    /* do point */
}

void app::host::paint()
{
    if (0/* has clients */)
        ; // send_paint();

    /* do paint */

    if (0/* has clients */)
    {
        /* wait reply */
    }

    if (0/* has server */)
    {
        /* send reply */
    }

    /* swap */
}

//-----------------------------------------------------------------------------
