//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef ORBITER_HPP
#define ORBITER_HPP

#include <vector>

#include <app-prog.hpp>

#include "sph-viewer.hpp"

//-----------------------------------------------------------------------------

class orbiter : public sph_viewer
{
public:

    orbiter(const std::string&, const std::string&);

    virtual void draw(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual ~orbiter();

private:

    void tick_move(double);
    void tick_look(double);
    void tick_dive(double);
    void tick(double);

    double orbit_plane[3];
    double orbit_speed;
    double position[3];
    double altitude;
    double view_x[3];
    double view_y[3];

    double point_v[3];
    double click_v[3];
    bool   drag_move;
    bool   drag_look;
    bool   drag_dive;
    bool   drag_light;

    bool pan_point(app::event *);
    bool pan_click(app::event *);
    bool pan_key  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
