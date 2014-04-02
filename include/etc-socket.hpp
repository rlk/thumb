//  Copyright (C) 2007-2011 Robert Kooima
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

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <list>

//-----------------------------------------------------------------------------
// These definitions homogenize Winsock with Berkeley Sockets. (needs work)

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>

#undef E_DRAW

typedef int   socklen_t;
typedef ULONG in_addr_t;

#define sock_errno   WSAGetLastError()
#define ECONNREFUSED WSAECONNREFUSED
#define usleep(t)    Sleep(t)

#else // not _WIN32 -----------------------------------------------------------

#define sock_errno errno

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>

typedef int SOCKET;
#define SOCKET_ERROR   -1
#define INVALID_SOCKET -1

#endif

typedef struct sockaddr_in sockaddr_t;

typedef std::list<SOCKET>           SOCKET_v;
typedef std::list<SOCKET>::iterator SOCKET_i;

//-----------------------------------------------------------------------------

// We avoid the use of full host name lookup because gethostbyname precludes
// static linking under Linux. However, inet_aton is not implemented by WinSock.
// Thus, we're left with the deprecated inet_addr.

inline bool init_sockaddr(sockaddr_t &dest, const char *name, short port)
{
    in_addr_t addr = in_addr_t(inet_addr(name));

    if (addr != INADDR_NONE)
    {
        dest.sin_family = AF_INET;
        dest.sin_port = htons(port);
        dest.sin_addr.s_addr = addr;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------

#endif
