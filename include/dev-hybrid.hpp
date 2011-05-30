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
        int  up;
        int  dn;
        bool upok;
        bool dnok;

    public:

        axis(app::node=0, int=-1, int=-1, int=-1, int=-1);

        void   process_button(int, bool);
        double process_axis  (int, int);
    };

    //-------------------------------------------------------------------------

    class button
    {
        int  index;
        int  up;
        int  dn;
        bool upok;
        bool dnok;

    public:

        button(app::node=0, int=-1, int=-1, int=-1);

        bool process_button(int, bool);
    };

    //-------------------------------------------------------------------------

    class hybrid : public input
    {
        // Configuration file

        app::file file;
        app::node node;

        // Configuration

        axis   look_LR;
        axis   look_UD;

        axis   move_LR;
        axis   move_FB;
        axis   move_UD;

        axis   hand_LR;
        axis   hand_FB;
        axis   hand_UD;

        button button_0;
        button button_1;
        button button_2;
        button button_3;
        button move_home;
        button hand_home;
        button peek_U;
        button peek_D;
        button peek_L;
        button peek_R;

        // Navigation state

        double position[3];
        double rotation[2];

        double hand_q[4];
        double hand_p[3];
        double hand_v[3];

        int depth;

        void synthesize_point();
        void recenter_hand();

        // Event handlers

        bool process_point (app::event *);
        bool process_button(app::event *);
        bool process_axis  (app::event *);
        bool process_tick  (app::event *);

    public:

        hybrid(const std::string&);

        bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
