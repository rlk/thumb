//  Copyright (C) 2005 Robert Kooima
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

#ifndef MODE_INFO_HPP
#define MODE_INFO_HPP

#include "mode-mode.hpp"

//-----------------------------------------------------------------------------

namespace cnt
{
    class control;
}

//-----------------------------------------------------------------------------

namespace mode
{
    class info : public mode
    {
        cnt::control *gui;

    public:

        info(wrl::world *);
       ~info();

        virtual ogl::range prep(int, app::frustum **);
        virtual void       draw(int, app::frustum  *);

        virtual bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
