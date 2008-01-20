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
#include "texture.hpp"
#include "message.hpp"
#include "varrier.hpp"
#include "frustum.hpp"
#include "matrix.hpp"
#include "serial.hpp"
#include "frame.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    //-------------------------------------------------------------------------
    // View

    class view
    {
        double V[3];
        double P[3];

        int w;
        int h;

        ogl::frame *back;

        float color[3];

    public:

        view(app::node, const int *);
       ~view();

        void set_head(const double *, const double *,
                      const double *, const double *);

        const double *get_P() const { return P; }

        void bind(GLenum t) const { back->bind_color(t); }
        void free(GLenum t) const { back->free_color(t); }

        void draw(const int *, bool);
    };

    //-------------------------------------------------------------------------
    // Tile

    class tile
    {
    private:

        int window[4];

        app::frustum *frustum;
        app::varrier *varrier;

        int view_index;
        int tile_index;

    public:

        tile(app::node);
       ~tile();

        bool input_point(int, const double *, const double *);
        bool input_click(int, int, int, bool);
        bool input_keybd(int, int, int, bool);

        void draw(std::vector<view *>&, int);

        bool pick(double *, double *, int, int);

        bool is_index(int i) const { return (i == tile_index); }
    };

    typedef std::vector<tile *>::iterator tile_i;

    //-------------------------------------------------------------------------
    // Host

    typedef std::vector<SOCKET>           SOCKET_v;
    typedef std::vector<SOCKET>::iterator SOCKET_i;

    class host
    {
        // Network handling

        void fork_client(const char *, const char *);
        void poll_listen();

        void init_listen(app::node);
        void init_server(app::node);
        void init_client(app::node);

        void fini_listen();
        void fini_server();
        void fini_client();

        SOCKET   server_sd;
        SOCKET   listen_sd;
        SOCKET_v client_sd;

        void send(message&);
        void recv(message&);

        // Event handlers

        void point(int, const double *, const double *);
        void click(int, int, int, bool);
        void keybd(int, int, int, bool);
        void value(int, int, double);
        void timer(int);
        void paint();
        void front();
        void close();

        // Event loops

        void root_loop();
        void node_loop();

        int tock;

        // Window config

        int window[4];
        int buffer[2];

        std::vector<view *> views;
        std::vector<tile *> tiles;

        // Config IO

        app::serial file;

        int current_index;

    public:

        host(std::string, std::string);
       ~host();

        bool root() const { return (server_sd == INVALID_SOCKET); }
        void loop();
        void draw();

        int get_window_x() const { return window[0]; }
        int get_window_y() const { return window[1]; }
        int get_window_w() const { return window[2]; }
        int get_window_h() const { return window[3]; }
        int get_window_m() const;
        int get_buffer_w() const { return buffer[0]; }
        int get_buffer_h() const { return buffer[1]; }

        void set_head(const double *, const double *,
                      const double *, const double *);
/*
        void gui_pick(int&, int&, const double *, const double *) const;
        void gui_size(int&, int&)                                 const;
        void gui_view()                                           const;
        void tag_draw();
*/
        bool tile_input_point(int, const double *, const double *);
        bool tile_input_click(int, int, int, bool);
        bool tile_input_keybd(int, int, int, bool);
    };
}

//-----------------------------------------------------------------------------

extern app::host *host;

//-----------------------------------------------------------------------------

#endif
