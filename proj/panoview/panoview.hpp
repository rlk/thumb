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

#ifndef PANOVIEW_HPP
#define PANOVIEW_HPP

#include <vector>

#include <app-prog.hpp>

#include "scm-viewer.hpp"

//-----------------------------------------------------------------------------

class panoview : public scm_viewer
{
public:

    panoview(const std::string&, const std::string&);

    virtual void draw(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual ~panoview();

private:

    double min_zoom;
    double max_zoom;
    bool  drag_zooming;
    bool  drag_looking;
    int   drag_x;
    int   drag_y;
    int   curr_x;
    int   curr_y;

    bool pan_point(app::event *);
    bool pan_click(app::event *);
    bool pan_tick (app::event *);
};

//-----------------------------------------------------------------------------

#endif
