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

#include "uni-overlay.hpp"
#include "util.hpp"

//-----------------------------------------------------------------------------

void uni::overlay::init_listen(int port)
{
    socklen_t  addresslen = sizeof (sockaddr_t);
    sockaddr_t address;

    address.sin_family      = AF_INET;
    address.sin_port        = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    // Create a socket, set no-delay, bind the port, and listen.

    if ((listen_sd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        throw std::runtime_error(strerror(sock_errno));
        
    nodelay(listen_sd);

    while (bind(listen_sd, (struct sockaddr *) &address, addresslen) < 0)
        if (sock_errno == EINVAL)
        {
            std::cerr << "Waiting for port expiration" << std::endl;
            usleep(250000);
        }
        else throw std::runtime_error(strerror(sock_errno));

    listen(listen_sd, 16);
}

void uni::overlay::poll_listen()
{
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
                if (sock_errno != EINTR)
                    throw std::runtime_error(strerror(sock_errno));
            }
            else
            {
                // Accept the connection.

                if (int sd = accept(listen_sd, 0, 0))
                {
                    if (sd < 0)
                        throw std::runtime_error(strerror(sock_errno));
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

void uni::overlay::fini_listen()
{
    if (listen_sd != INVALID_SOCKET)
        ::close(listen_sd);

    listen_sd = INVALID_SOCKET;
}

//-----------------------------------------------------------------------------

void uni::overlay::step_client(SOCKET_i i)
{
}

void uni::overlay::poll_client()
{
    fd_set fds;
    int m = 0;

    // Initialize the socket descriptor set with all connected sockets.

    FD_ZERO(&fds);

    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
    {
        if (*i != INVALID_SOCKET)
            FD_SET(*i, &fds);
        m = MAX(m, *i);
    }

    // Check for activity on all sockets.

    if (int n = select(m + 1, &sd, NULL, NULL, &tv))
    {
        if (n < 0)
        {
            if (sock_errno != EINTR)
                throw std::runtime_error(strerror(sock_errno));
        }
        else
        {
            for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
                if (*i != INVALID_SOCKET && FD_ISSET(*i, &fds))
                    recv_client(*i);
        }
    }
}

void uni::overlay::fini_client()
{
    for (SOCKET_i i = client_sd.begin(); i != client_sd.end(); ++i)
    {
        char c;

        // Wait for EOF (orderly shutdown by the remote).

        while (::recv(*i, &c, 1, 0) > 0)
            ;

        // Close the socket.

        ::close(*i);
    }
}

//-----------------------------------------------------------------------------

void uni::overlay::step()
{
    poll_listen();
    poll_client();
}

//-----------------------------------------------------------------------------

uni::overlay::overlay(int port)
{
    init_listen(port);
}

uni::overlay::~overlay()
{
    fini_listen();
}

//-----------------------------------------------------------------------------
