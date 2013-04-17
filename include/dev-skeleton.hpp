//  Copyright (C) 2013 Robert Kooima
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

#ifndef DEV_SKELETON
#define DEV_SKELETON

#include <vector>

#include <dev-input.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    class skeleton : public input
    {
        int    port;
        SOCKET sock;

        void step();

    public:

        skeleton();
       ~skeleton();

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
