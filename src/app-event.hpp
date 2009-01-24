//  Copyright (C) 2008 Robert Kooima
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

#ifndef APP_EVENT_HPP
#define APP_EVENT_HPP

#include <string>
#include <cstring>
#include <stdexcept>
#include <errno.h>

#include "socket.hpp"

// TODO: Flag and cache the encoded state.  Don't re-encode.
// TODO: Allow response string.

//-----------------------------------------------------------------------------

#define DATAMAX 256

// Event types

#define E_NULL  0
#define E_REPLY 1
#define E_POINT 2
#define E_CLICK 3
#define E_KEYBD 4
#define E_VALUE 5
#define E_INPUT 6
#define E_TIMER 7
#define E_PAINT 8
#define E_FRAME 9
#define E_START 10
#define E_CLOSE 11

// Event responses

#define R_IGNORED 0
#define R_HANDLED 1
#define R_DRAW    2
#define R_EXIT    4

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------

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

    //-------------------------------------------------------------------------

    class event
    {
        // Network IO buffer

        union swap { double d; int i; uint32_t l[2]; };

        struct { unsigned char type;
                 unsigned char size;
                 char          data[DATAMAX]; } payload;

        int index;

        // Data marshalling functions

        const char   *get_text();
        double        get_real();
        bool          get_bool();
        int           get_byte();
        int           get_word();

        void          put_text(const char *);
        void          put_real(double);
        void          put_bool(bool);
        void          put_byte(int);
        void          put_word(int);

    public:

        event();
       ~event();

        // Type-specific payloads

        struct point_data_s
        {
            int    i;
            double p[3];
            double q[4];
        };
        struct click_data_s
        {
            int    i;
            int    b;
            int    m;
            bool   d;
        };
        struct keybd_data_s
        {
            int    c;
            int    k;
            int    m;
            bool   d;
        };
        struct value_data_s
        {
            int    i;
            int    a;
            double v;
        };
        struct input_data_s
        {
            char *src;
            char *dst;
        };
        struct timer_data_s
        {
            int dt;
        };

        // Payload union

        union {
            point_data_s point;
            click_data_s click;
            keybd_data_s keybd;
            value_data_s value;
            input_data_s input;
            timer_data_s timer;
        } data;

        void          put_type(unsigned char);
        unsigned char get_type();

        // Event builders

        event *mk_point(int, const double *, const double *);
        event *mk_click(int, int, int, bool);
        event *mk_keybd(int, int, int, bool);
        event *mk_value(int, int, double);
        event *mk_input(const char *);
        event *mk_timer(int);
        event *mk_paint();
        event *mk_frame();
        event *mk_start();
        event *mk_close();

        // Network IO

        event *send(SOCKET);
        event *recv(SOCKET);
    };
}

//-----------------------------------------------------------------------------

#endif