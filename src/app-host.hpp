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

#ifndef APP_HOST_HPP
#define APP_HOST_HPP

#include <vector>
#include <string>

#include "socket.hpp"
#include "app-file.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class event;
    class frustum;
}

namespace dpy
{
    class channel;
    class display;
}

//-----------------------------------------------------------------------------

namespace app
{
    class host
    {
        // Network handling

        void   fork_client(const char *, const char *, const char *);
        void   poll_listen(bool);
        void   poll_script();
        void   fini_script();

        SOCKET init_socket(int);
        void   init_listen(app::node);
        void   init_server(app::node);
        void   init_client(app::node);

        void   fini_listen();
        void   fini_server();
        void   fini_client();

        int      clients;

        SOCKET   server_sd;
        SOCKET   client_cd;
        SOCKET_v client_sd;
        SOCKET   script_cd;
        SOCKET_v script_sd;

        void send(event *);
        void sync();

        // Event loops

        bool draw_flag;
        bool exit_flag;

        void root_loop();
        void node_loop();

        int  tock;
        int  mode;
        int  bench;
        int  movie;
        int  count;

        // Event/Calibration handlers

        bool calibration_state;
        int  calibration_index;

        bool process_calib(event *E);
        void process_start(event *E);
        void process_close(event *E);

        // Window config

        int window_full;
        int window_frame;
        int window_size[4];
        int buffer_size[2];

        std::vector<dpy::display *> displays;
        std::vector<dpy::channel *> channels;
        std::vector<app::frustum *> frustums;

        app::frustum *overlay;

        // Configuration serializer

        app::file file;

    public:

        host(std::string, std::string);
       ~host();

        bool root() const { return (server_sd == INVALID_SOCKET); }
        void loop();
        void draw();
        void swap() const;

        bool pointer_to_3D(event *, int, int);
        bool process_event(event *);

        // Configuration queries.

        int get_window_m() const;
        int get_window_x() const { return window_size[0]; }
        int get_window_y() const { return window_size[1]; }
        int get_window_w() const { return window_size[2]; }
        int get_window_h() const { return window_size[3]; }
        int get_buffer_w() const { return buffer_size[0]; }
        int get_buffer_h() const { return buffer_size[1]; }

        const app::frustum *get_overlay() const { return overlay; }

        void set_head(const double *, const double *);

        bool get_movie_mode() const { return movie; }
        void set_movie_mode(int i)  { movie = i;    }
        bool get_bench_mode() const { return bench; }
        void set_bench_mode(int i)  { bench = i;    }

        void post_draw() { draw_flag = true; }
        void post_exit() { exit_flag = true; }
    };
}

//-----------------------------------------------------------------------------

extern app::host *host;

//-----------------------------------------------------------------------------

#endif
