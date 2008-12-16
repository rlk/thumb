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

#ifndef UNI_OVERLAY_HPP
#define UNI_OVERLAY_HPP

#include <vector>

#include "socket.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    // TODO: Remove duplication with app-host.hpp

    typedef std::vector<SOCKET>           SOCKET_v;
    typedef std::vector<SOCKET>::iterator SOCKET_i;

    class overlay
    {
        SOCKET   listen_sd;
        SOCKET_v client_sd;

        void init_listen(int);
        void poll_listen();
        void fini_listen();

        void step_client(SOCKET_i);
        void poll_client();
        void fini_client();

    public:

        overlay(int);
       ~overlay();

        void step();

        void draw_images();
        void draw_models();
    };
}

//-----------------------------------------------------------------------------

#endif
