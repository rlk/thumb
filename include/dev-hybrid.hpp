//  Copyright (C) 2010-2011 Robert Kooima
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

#ifndef DEV_HYBRID
#define DEV_HYBRID

#include <dev-input.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace dev
{
    //-------------------------------------------------------------------------

    class axis
    {
        int  paxis;
        int  naxis;
        int  pval;
        int  nval;
        int  mod;
        bool active;

    public:

        axis(app::node=0, int=-1, int=-1, int=-1);

        void   click(int, bool);
        double value(int, int);
    };

    //-------------------------------------------------------------------------

    class button
    {
        int  index;
        int  mod;
        bool active;

    public:

        button(app::node=0, int=-1, int=-1);

        bool click(int, bool);
    };

    //-------------------------------------------------------------------------

    class hybrid : public input
    {
        // Configuration file

        app::file file;
        app::node node;

        // Configuration

        axis move_LR;
        axis move_FB;
        axis move_UD;

        axis look_LR;
        axis look_UD;

        axis hand_LR;
        axis hand_FB;
        axis hand_UD;

        button use;
        button move_home;
        button hand_home;
        button peek_U;
        button peek_D;
        button peek_L;
        button peek_R;

        // Navigation state

        double position[3];
        double rotation[2];

        // Event handlers

        bool process_point(app::event *);
        bool process_click(app::event *);
        bool process_axis(app::event *);
        bool process_tick(app::event *);

    public:

        hybrid(const std::string&);

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
