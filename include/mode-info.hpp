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

#ifndef MODE_INFO_HPP
#define MODE_INFO_HPP

#include <mode-mode.hpp>

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
        int gui_w;
        int gui_h;

        int pane;

        cnt::control *gui;

        void gui_show();
        void gui_hide();

    public:

        info(wrl::world *);
       ~info();

        virtual ogl::aabb prep(int, const app::frustum *const *);
        virtual void      draw(int, const app::frustum *);

        virtual bool process_event(app::event *);
    };
}

//-----------------------------------------------------------------------------

#endif
