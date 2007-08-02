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

#ifndef HOST_HPP
#define HOST_HPP

#include <string>
#include <vector>
#include <mxml.h>

#include "socket.hpp"

#define DEFAULT_HOST_FILE "host.xml"
#define DEFAULT_HOST      "localhost"
#define DEFAULT_PORT      "2827"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // Message handler

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
        void   put_bool  (bool);
        void   put_int   (int);

        double get_double();
        bool   get_bool  ();
        int    get_int   ();

        // Network IO

        void send(SOCKET);
        void recv(SOCKET);

        unsigned char type() const { return payload.type; }
    };

    //-------------------------------------------------------------------------
    // Host

    typedef std::vector<SOCKET>           SOCKET_v;
    typedef std::vector<SOCKET>::iterator SOCKET_i;

    class host
    {
        unsigned long lookup(const char *);

        void init_server();
        void init_client();
        void fini_server();
        void fini_client();

        SOCKET   server;
        SOCKET_v client;

        int tock;

        mxml_node_t *head;
        mxml_node_t *node;

        void send(message&);
        void recv(message&);

        // Event handlers

        void track(int, const double *, const double *);
        void stick(int, const double *);
        void point(int, int);
        void click(int, bool);
        void keybd(int, bool);
        void timer(int);
        void paint();
        void close();

        // Event loops

        void root_loop();
        void node_loop();

        // Config IO

        void load(std::string);

    public:

        host(std::string);
       ~host();

        void loop();
    };
}

//-----------------------------------------------------------------------------

#endif
