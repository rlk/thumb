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

#ifndef PANOPTIC_HPP
#define PANOPTIC_HPP

#include <vector>

#include <app-prog.hpp>
#include <etc-socket.hpp>

#include "view-app.hpp"

//-----------------------------------------------------------------------------

class panoptic : public view_app
{
public:

    panoptic(const std::string&, const std::string&);

    virtual ogl::range prep(int, const app::frustum * const *);
    virtual void       draw(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual void load_file(const std::string&);

    virtual void fade_to(int);

    virtual ~panoptic();

private:

    double get_speed() const;
    double get_scale() const;

    // Configuration

    double orbiter_speed_min;
    double orbiter_speed_max;
    double orbiter_minimum_agl;

    double panoview_zoom_min;
    double panoview_zoom_max;

    double now;
    double delta;

    // Joystick state

    double axis  [8];
    bool   button[16];

    double deaden(double) const;
    void joystick(double);

    // Joystick configuration

    int    device;
    double deadzone;

    int    orbiter_axis_rotate;
    int    orbiter_axis_forward;
    int    orbiter_axis_left;
    int    orbiter_axis_right;
    int    orbiter_button_up;
    int    orbiter_button_down;

    int    panoview_axis_vertical;
    int    panoview_axis_horizontal;
    int    panoview_button_in;
    int    panoview_button_out;

    // Report stream configuration

    sockaddr_in report_addr;
    SOCKET      report_sock;
    void        report();

    // Event handlers

    bool process_axis  (app::event *);
    bool process_button(app::event *);
    bool process_point (app::event *);
    bool process_click (app::event *);
    bool process_tick  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
