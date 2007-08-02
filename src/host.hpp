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
    // Host exceptions

    class name_error : public std::runtime_error
    {
        std::string mesg(const char *n) {
            std::string s(n);
            return "Error looking up host: " + s + ": " + hstrerror(h_errno);
        }

    public:
        name_error(const char *n) : std::runtime_error(mesg(n)) { }
    };

    //-------------------------------------------------------------------------
    // Host

    class host
    {
        unsigned long lookup(const char *);

        SOCKET              server;
        std::vector<SOCKET> client;

        mxml_node_t *head;
        mxml_node_t *node;

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
