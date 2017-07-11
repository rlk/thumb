
//  Copyright (C) 2007-2011 Robert Kooima
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

#include <cassert>
#include <cstdio>

#include <etc-vector.hpp>
#include <app-glob.hpp>
#include <app-host.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <ogl-program.hpp>
#include <dpy-channel.hpp>
#include <dpy-null.hpp>

//-----------------------------------------------------------------------------

dpy::null::null(app::node p) : display(p)
{
}

dpy::null::~null()
{
}

//-----------------------------------------------------------------------------

int dpy::null::get_frusc() const
{
    return 0;
}

void dpy::null::get_frusv(app::frustum **frusv) const
{
}

//-----------------------------------------------------------------------------

void dpy::null::prep(int chanc, const dpy::channel *const *chanv)
{
}

void dpy::null::draw(int chanc, const dpy::channel *const *chanv, int frusi)
{
}

void dpy::null::test(int chanc, const dpy::channel *const *chanv, int index)
{
}

//-----------------------------------------------------------------------------

bool dpy::null::pointer_to_3D(app::event *E, int x, int y)
{
    return false;
}

//-----------------------------------------------------------------------------
