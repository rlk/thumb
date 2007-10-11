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

#include "default.hpp"
#include "socket.hpp"
#include "frame.hpp"

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

        const char *tag() const;

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
    // Eye

    class eye
    {
        double V[3];
        double P[3];

        int w;
        int h;

        ogl::frame *back;

        unsigned char color[3];

    public:

        eye(mxml_node_t *, int, int);
       ~eye();

        void set_head(const double *, const double *,
                      const double *, const double *);

        const double *get_P() const { return P; }

        void bind(GLenum t) const { back->bind_color(t); }
        void free(GLenum t) const { back->free_color(t); }

        void draw(const int *);
    };

    //-------------------------------------------------------------------------
    // Tile

    class tile
    {
    private:

        double BL[3];
        double BR[3];
        double TL[3];
        double TR[3];

        double W;
        double H;

        int window_rect[4];

        double varrier[5];

    public:

        tile(mxml_node_t *);

        void draw(std::vector<eye *>&);

        bool pick(double *, double *, int, int);

        const double *get_BL() const { return BL; }
        const double *get_BR() const { return BR; }
        const double *get_TL() const { return TL; }
        const double *get_TR() const { return TR; }
    };

    //-------------------------------------------------------------------------
    // Host

    typedef std::vector<SOCKET>           SOCKET_v;
    typedef std::vector<SOCKET>::iterator SOCKET_i;

    class host
    {
        // Network handling

        void fork_client(const char *, const char *);
        void poll_listen();

        void init_listen();
        void init_server();
        void init_client();

        void fini_listen();
        void fini_server();
        void fini_client();

        SOCKET   server_sd;
        SOCKET   listen_sd;
        SOCKET_v client_sd;

        int tock;
        int mods;

        void send(message&);
        void recv(message&);

        // Event handlers

        void track(int, const double *, const double *, const double *);
        void stick(int, const double *);
        void point(const double *, const double *);
        void click(int, bool);
        void keybd(int, int, int, bool);
        void timer(int);
        void paint();
        void fleep();
        void close();

        // Event loops

        void root_loop();
        void node_loop();

        // GUI config

        double gui_M[16];
        double gui_I[16];
        int    gui_w;
        int    gui_h;

        // Window config

        int window_rect[4];
        int buffer_w;
        int buffer_h;

        std::vector<eye *>  eyes;
        std::vector<tile> tiles;

        // Config IO

        mxml_node_t *head;
        mxml_node_t *node;

        void load(std::string&, std::string&);

    public:

        host(std::string, std::string);
       ~host();

        bool root() const { return (server_sd == INVALID_SOCKET); }
        void loop();
        void draw();

        int  get_frustum(double *) const;
        bool get_plane  (double *, const double *,
                                   const double *,
                                   const double *,
                                   const double *, int) const;

        int get_window_x() const { return window_rect[0]; }
        int get_window_y() const { return window_rect[1]; }
        int get_window_w() const { return window_rect[2]; }
        int get_window_h() const { return window_rect[3]; }
        int get_window_m() const;

        int get_buffer_w() const { return buffer_w; }
        int get_buffer_h() const { return buffer_h; }

        int modifiers() const { return mods; }

        void set_head(const double *, const double *,
                      const double *, const double *);

        void gui_pick(int&, int&, const double *, const double *) const;
        void gui_size(int&, int&)                                 const;
        void gui_view()                                           const;
    };
}

//-----------------------------------------------------------------------------

extern app::host *host;

//-----------------------------------------------------------------------------

#endif
