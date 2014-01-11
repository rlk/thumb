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

#include <etc-vector.hpp>
#include <etc-socket.hpp>
#include <app-conf.hpp>
#include <app-view.hpp>
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

    vec3 p;
    quat q;

    if (sock != INVALID_SOCKET)
    {
        fd_set readfds;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        while (select(sock + 1, &readfds, NULL, NULL, &zero) > 0)
        {
            if (FD_ISSET(sock, &readfds))
            {
                if (recv(sock, (char *) data, sizeof data, 0) == sizeof data)
                {
                    vec3 x(1, 0, 0);
                    vec3 y(0, 1, 0);
                    vec3 z(0, 0, 1);

                    // Compute the eyes midpoint in meters.

                    p[0] = double(data[3] + data[0]) * 0.0254;
                    p[1] = double(data[4] + data[1]) * 0.0254;
                    p[2] = double(data[5] + data[2]) * 0.0254;

                    // Compute the head orientation.

                    x[0] = double(data[3] - data[0]);
                    x[1] = double(data[4] - data[1]);
                    x[2] = double(data[5] - data[2]);

                    x = normal(x);
                    z = normal(cross(x, y));
                    y = normal(cross(z, x));

                    ::host->set_head(p, quat(mat3(x[0], y[0], z[0],
                                                  x[1], y[1], z[1],
                                                  x[2], y[2], z[2])));
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
