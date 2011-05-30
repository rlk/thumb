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

#ifndef MODE_PLAY_HPP
#define MODE_PLAY_HPP

#include <mode-mode.hpp>

//-----------------------------------------------------------------------------

namespace mode
{
    class play : public mode
    {
        bool process_start(app::event *);
        bool process_close(app::event *);
        bool process_tick(app::event *);

    public:

        play(wrl::world *);

        virtual bool process_event(app::event *);

        virtual ~play();
    };
}

//-----------------------------------------------------------------------------

#endif
