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

    virtual void load_file(const std::string&);

    virtual int move_to(int);
    virtual int fade_to(int);

    virtual void navigate(const double *);
    virtual void get_up_vector(double *);

    virtual ~orbiter();

private:

    // View motion state

    double get_speed() const;

    double speed_min;
    double speed_max;
    double stick_timer;
    double minimum_agl;

    // Report stream configuration

    sockaddr_in report_addr;
    SOCKET      report_sock;
    void        report();

    // Event handlers

    bool process_tick  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
