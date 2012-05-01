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

#include "scm-viewer.hpp"

//-----------------------------------------------------------------------------

class orbiter : public scm_viewer
{
public:

    orbiter(const std::string&, const std::string&);

    virtual ogl::range prep(int, const app::frustum * const *);
    virtual void       draw(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual void load(const std::string&);

    virtual ~orbiter();

private:

    void tick_move (double);
    void tick_look (double);
    void tick_dive (double);
    void tick_light(double);

    struct state
    {
        double orbit_plane[3];
        double orbit_speed;
        double position[3];
        double altitude;
        double view_x[3];
        double view_y[3];
        double light[3];

        state();

        double scale(double);

        void move(const double *, const double *, double, double);
        void look(const double *, const double *, double, double);
        void turn(const double *, const double *, double, double);
        void dive(const double *, const double *, double, double);
        void lite(const double *, const double *, double, double);

        void update(double, double *);
    };

    void loadstate();
    void savestate();

    state  current;
    state  saved[12];

    double point[3];
    double click[3];
    bool   control;
    bool   drag_move;
    bool   drag_look;
    bool   drag_turn;
    bool   drag_dive;
    bool   drag_lite;

    bool pan_point(app::event *);
    bool pan_click(app::event *);
    bool pan_tick (app::event *);
    bool pan_key  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
