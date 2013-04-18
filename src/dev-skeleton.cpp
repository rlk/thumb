//  Copyright (C) 2013 Robert Kooima
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

#include <cstdio>
#include <cassert>

#include <etc-socket.hpp>
#include <etc-math.hpp>
#include <app-conf.hpp>
#include <app-user.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <dev-skeleton.hpp>

//-----------------------------------------------------------------------------

dev::skeleton::skeleton() : port(0), sock(INVALID_SOCKET)
{
    if ((port = ::conf->get_i("skeleton_port", 4953)))
    {
        if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) >= 0)
        {
            socklen_t  len = sizeof (sockaddr_t);
            sockaddr_t addr;

            // Accept connections from any address on the configured port.

            addr.sin_family      = AF_INET;
            addr.sin_port        = htons(uint16_t(port));
            addr.sin_addr.s_addr = htonl(uint32_t(INADDR_ANY));

            /* Bind the socket to this address. */

            if (bind(sock, (struct sockaddr *) &addr, len) >= 0)
            {
            }
            else throw app::sock_error("bind");
        }
        else throw app::sock_error("socket");
    }
}

dev::skeleton::~skeleton()
{
    if (sock != INVALID_SOCKET) close(sock);
}

//-----------------------------------------------------------------------------

void dev::skeleton::step()
{
    struct timeval zero = { 0, 0 };
    float data[6];
    
    double p[3] = { 0.0, 0.0, 0.0 };
    double q[4] = { 0.0, 0.0, 0.0, 1.0 };

    if (sock != INVALID_SOCKET)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        while (select(sock + 1, &readfds, NULL, NULL, &zero) > 0)
        {
            if (FD_ISSET(sock, &readfds))
            {
                if (recv(sock, data, sizeof (data), 0) == sizeof (data))
                {
                    double M[16];
                    double x[3];
                    double y[3] = { 0.0, 1.0, 0.0 };
                    double z[3];

                    // Compute the eyes midpoint in meters.

                    p[0] = double(data[3] + data[0]) * 0.0254;
                    p[1] = double(data[4] + data[1]) * 0.0254;
                    p[2] = double(data[5] + data[2]) * 0.0254 - 1.5;

                    // Compute the head orientation quaternion.

                    x[0] = double(data[3] - data[0]);
                    x[1] = double(data[4] - data[1]);
                    x[2] = double(data[5] - data[2]);

                    normalize(x);
                    crossprod(z, x, y);
                    normalize(z);
                    crossprod(y, z, x);
                    normalize(y);
                    set_basis(M, x, y, z);

                    mat_to_quat(q, M);

                    ::host->set_head(p, q);
                }
            }
        }
    }
    else ::host->set_head(p, q);
}

//-----------------------------------------------------------------------------

bool dev::skeleton::process_event(app::event *E)
{
    assert(E);

    if (E->get_type() == E_TICK)
        step();

    return dev::input::process_event(E);
}

//-----------------------------------------------------------------------------
