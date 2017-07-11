//  Copyright (C) 2007-2011,2017 Robert Kooima
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

#ifndef DPY_NULL_HPP
#define DPY_NULL_HPP

#include <dpy-display.hpp>
#include <app-file.hpp>

//-----------------------------------------------------------------------------

namespace dpy
{
    class null : public display
    {
    public:

        null(app::node);

        virtual ~null();

        // Frustum queries

        virtual int  get_frusc()                const;
        virtual void get_frusv(app::frustum **) const;

        virtual app::frustum *get_overlay() const { return 0; }

        // Rendering handlers

        virtual void prep(int, const dpy::channel *const *);
        virtual void draw(int, const dpy::channel *const *, int);
        virtual void test(int, const dpy::channel *const *, int);

        // Event handers

        virtual bool pointer_to_3D(app::event *, int, int);
    };
}

//-----------------------------------------------------------------------------

#endif
