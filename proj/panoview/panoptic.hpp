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

    // View motion state

    void fly (double);

    double orbit_plane[3];
    double orbit_speed;
    double orbit_speed_min;
    double orbit_speed_max;
    double minimum_agl;

    double now;
    double delta;

    // Interaction state

    double stick[2];
    bool   fly_dn;
    bool   fly_up;

    // Joystick configuration

    int    device;
    int    axis_X;
    int    axis_Y;
    int    button_U;
    int    button_D;
    double deadzone;

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
