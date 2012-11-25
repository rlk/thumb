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
#include <etc-socket.hpp>

#include "view-app.hpp"

//-----------------------------------------------------------------------------

class orbiter : public view_app
{
public:

    orbiter(const std::string&, const std::string&);

    virtual ogl::range prep(int, const app::frustum * const *);
    virtual void       draw(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual void load(const std::string&);

    virtual double get_scale() const;
    virtual void   make_path(int);

    virtual ~orbiter();

private:

    double get_bottom() const;

    // View motion state

    void look(double, double);
    void lite(double, double);
    void move(double, double);
    void dive(double, double);
    void fly (double);

    double orbit_plane[3];
    double orbit_speed;
    double stick_timer;
    double goto_radius;
    
    double dist_near;
    double dist_far;
    
    // Interaction state

    double point[3];
    double click[3];
    double stick[3];
    bool   control;
    bool   drag_move;
    bool   drag_look;
    bool   drag_turn;
    bool   drag_dive;
    bool   drag_lite;

    // Joystick configuration

    int    device;
    int    axis_X;
    int    axis_Y;
    int    button_U;
    int    button_D;
    double deadzone;

    // Report stream configuration

    sockaddr_in report_addr;
    int         report_sock;
    void        report();

    // Event handlers

    bool pan_axis  (app::event *);
    bool pan_button(app::event *);
    bool pan_point (app::event *);
    bool pan_click (app::event *);
    bool pan_tick  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
