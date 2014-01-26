//  Copyright (C) 2005-2011 Robert Kooima
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

#ifndef APP_PROG_HPP
#define APP_PROG_HPP

#include <string>
#include <vector>

#include <SDL.h>

#include <ogl-aabb.hpp>
#include <etc-vector.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class event;
    class frustum;
}

namespace dev
{
    class input;
}

//-----------------------------------------------------------------------------

namespace app
{
    // Application interface.

    class prog
    {
    public:

        prog(const std::string&, const std::string&);

        virtual ~prog();

        bool is_running() const { return running; }

        void set_host_config(std::string);

        // Rendering handlers

        virtual ogl::aabb prep(int, const app::frustum * const *) = 0;
        virtual void      lite(int, const app::frustum * const *) = 0;
        virtual void      draw(int, const app::frustum *, int)    = 0;

        // Event handler

        virtual bool process_event(event *);
        virtual void run();
        virtual void stop();
        virtual void swap();

        virtual quat get_orientation() const;
        virtual void set_orientation(const quat&);
        virtual void offset_position(const vec3&);

        event *axis_remap(event *);

        // Screenshot procedure

        void screenshot(std::string, int, int);

    private:

        std::string host_config;
        std::string exe;
        
        bool running;
        bool restart;

        int key_snap;
        int key_init;

        SDL_Window   *window;
        SDL_GLContext context;
        SDL_Joystick *joystick;

        std::vector<short> axis_min;
        std::vector<short> axis_max;
        bool               axis_verbose;

        void video_up();
        void video_dn();

        void host_up(std::string);
        void host_dn();

        void axis_setup();
        void axis_state();

        dev::input *input;
        dev::input *mouse;

        unsigned char *snap_p;
        int            snap_w;
        int            snap_h;
    };
}

//-----------------------------------------------------------------------------

#endif
