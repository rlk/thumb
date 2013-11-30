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

#ifndef APP_HOST_HPP
#define APP_HOST_HPP

#include <vector>
#include <string>

#include <etc-vector.hpp>
#include <etc-socket.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class prog;
    class event;
    class frustum;
}

namespace dpy
{
    class channel;
    class display;
}

namespace ogl
{
    class frame;
}

//-----------------------------------------------------------------------------

namespace app
{
    class host
    {
    public:

        host(app::prog *, std::string, std::string, std::string);
       ~host();

        bool root() const { return (server_sd == INVALID_SOCKET); }
        void loop();
        void draw(int, const app::frustum *, int);
        void draw();
        void swap() const;

        bool pointer_to_3D(event *, int, int);
        bool process_event(event *);

        // Configuration queries.

        int get_window_m() const;
        int get_window_c() const { return window_cursor;  }
        int get_window_x() const { return window_rect[0]; }
        int get_window_y() const { return window_rect[1]; }
        int get_window_w() const { return window_rect[2]; }
        int get_window_h() const { return window_rect[3]; }
        int get_buffer_w() const { return buffer_size[0]; }
        int get_buffer_h() const { return buffer_size[1]; }
        int get_device()   const { return device; }

        const app::frustum *get_overlay() const { return overlay; }

        void set_head(const vec3&, const quat&);

        quat get_orientation() const;
        vec3 get_position   () const;
        void set_orientation(const quat&);
        void set_position   (const vec3&);

        bool get_movie_mode() const { return (movie != 0); }
        void set_movie_mode(int i)  { movie = i; }
        bool get_bench_mode() const { return (bench != 0); }
        void set_bench_mode(int i)  { bench = i; }

    private:

        // Network handling

        SOCKET init_socket(int, int);

        void   init_listen(app::node);
        void   fini_listen();
        void   poll_listen(bool);

        void   init_script();
        void   fini_script();
        void   poll_script();

        void   init_server(app::node);
        void   fini_server();

        void   init_client(app::node, const std::string&);
        void   fini_client();
        void   fork_client(const char *, const char *,
                           const char *, const char *);

        SOCKET   listen_sd;
        SOCKET   script_sd;
        SOCKET   server_sd;
        SOCKET_v client_sd;

        int      clients;

        void send(event *);
        void sync();

        // Event loops

        void root_loop();
        void node_loop();

        double tock;
        int    bench;
        int    movie;
        int    count;

        // Event/Calibration handlers

        bool calibration_state;
        int  calibration_index;

        bool process_calib(event *E);
        void process_start(event *E);
        void process_close(event *E);

        // Window config

        int window_full;
        int window_frame;
        int window_cursor;
        int window_rect[4];
        int buffer_size[2];
        int render_size[2];
        int device;

        std::vector<dpy::display *> displays;
        std::vector<dpy::channel *> channels;
        std::vector<app::frustum *> frustums;

        app::frustum *overlay;
        app::prog    *program;
        ogl::frame   *render;

        // Configuration serializer

        app::file file;
    };
}

//-----------------------------------------------------------------------------

extern app::host *host;

//-----------------------------------------------------------------------------

#endif
