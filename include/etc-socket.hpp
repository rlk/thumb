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

typedef int socklen_t;

#define ECONNREFUSED WSAECONNREFUSED
#define sock_errno   WSAGetLastError()
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

#endif //----------------------------------------------------------------------

typedef struct sockaddr_in sockaddr_t;

typedef std::list<SOCKET>           SOCKET_v;
typedef std::list<SOCKET>::iterator SOCKET_i;

//-----------------------------------------------------------------------------

#endif
