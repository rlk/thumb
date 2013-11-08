//  Copyright (C) 2008-2011 Robert Kooima
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
#include <stdint.h>
#include <errno.h>
#include <stdexcept>

#include <etc-socket.hpp>

//-----------------------------------------------------------------------------

#define DATAMAX 128

// Event types

#define E_NULL   0
#define E_POINT  1
#define E_CLICK  2
#define E_KEY    3
#define E_AXIS   4
#define E_BUTTON 5
#define E_TICK   6
#define E_DRAW   7
#define E_SWAP   8
#define E_USER   9
#define E_START 10
#define E_CLOSE 11
#define E_FLUSH 12

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------

    class host_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
#ifdef WIN32
			return s + ": " + "Host error";
#else
            return s + ": " + hstrerror(h_errno);
#endif
        }

    public:
        host_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    class sock_error : public std::runtime_error
    {
        std::string mesg(const std::string& s) {
            return s + ": " + strerror(errno);
        }

    public:
        sock_error(const std::string& s) : std::runtime_error(mesg(s)) { }
    };

    //-------------------------------------------------------------------------

    class event
    {
        // Network payload buffer

        struct { unsigned char type;
                 unsigned char size;
                 char          data[DATAMAX]; } payload;

        bool payload_cache;
        int  payload_index;

        void payload_encode();
        void payload_decode();

        // Data marshalling functions

        double      get_real();
        bool        get_bool();
        int         get_byte();
        int         get_word();
        long long   get_long();

        void        put_real(double);
        void        put_bool(bool);
        void        put_byte(int);
        void        put_word(int);
        void        put_long(long long);

    public:

        event();
       ~event();

        // Type-specific data

        struct point_data_t
        {
            int    i;
            double p[3];
            double q[4];
        };
        struct click_data_t
        {
            int    b;
            int    m;
            bool   d;
        };
        struct key_data_t
        {
            int    c;
            int    k;
            int    m;
            bool   d;
        };
        struct axis_data_t
        {
            int    i;
            int    a;
            double v;
        };
        struct button_data_t
        {
            int    i;
            int    b;
            bool   d;
        };
        struct user_data_t
        {
            long long d;
        };
        struct tick_data_t
        {
            double dt;
        };

        // Data union

        union {
             point_data_t point;
             click_data_t click;
               key_data_t key;
              axis_data_t axis;
            button_data_t button;
              user_data_t user;
              tick_data_t tick;
        } data;

        void          put_type(unsigned char);
        unsigned char get_type() const;

        // Event builders

        event *mk_point (int, const double *, const double *);
        event *mk_click (int, int, bool);
        event *mk_key   (int, int, int, bool);
        event *mk_axis  (int, int, double);
        event *mk_button(int, int, bool);
        event *mk_tick  (double);
        event *mk_draw  ();
        event *mk_swap  ();
        event *mk_user  (long long);
        event *mk_start ();
        event *mk_close ();
        event *mk_flush ();

        // Network IO

        event *send(SOCKET);
        event *recv(SOCKET);

        std::string name();
    };
}

//-----------------------------------------------------------------------------

#endif
