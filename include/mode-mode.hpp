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

#ifndef MODE_MODE_HPP
#define MODE_MODE_HPP

#include <ogl-range.hpp>

//-----------------------------------------------------------------------------

namespace app
{
    class event;
    class frustum;
}

namespace wrl
{
    class world;
}

//-----------------------------------------------------------------------------

namespace mode
{
    class mode
    {
    protected:

        wrl::world* world;

    public:

        mode(wrl::world *w) : world(w) { }

        virtual ogl::range prep(int, const app::frustum *const *);
        virtual void       lite(int, const app::frustum *const *);
        virtual void       draw(int, const app::frustum *);

        virtual bool process_event(app::event *) { return false; }

        virtual ~mode() { }
    };
}

//-----------------------------------------------------------------------------

#endif
