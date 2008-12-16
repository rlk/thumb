//  Copyright (C) 2005 Robert Kooima
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

#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "util.hpp"

#include "socket.hpp" // for ntohl()

//-----------------------------------------------------------------------------

float ntohf(float src)
{
    float dst;

    uint32_t *s = (uint32_t *) (&src);
    uint32_t *d = (uint32_t *) (&dst);

    *d = ntohl(*s);

    return dst;
}

int ntohi(int src)
{
    int dst;

    uint32_t *s = (uint32_t *) (&src);
    uint32_t *d = (uint32_t *) (&dst);

    *d = ntohl(*s);

    return dst;
}

//-----------------------------------------------------------------------------

void nodelay(int sd)
{
    socklen_t len = sizeof (int);
    int       val = 1;
        
    if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (const char *) &val, len) < 0)
        throw std::runtime_error(strerror(sock_errno));
}

//-----------------------------------------------------------------------------

int next_power_of_2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

//-----------------------------------------------------------------------------
