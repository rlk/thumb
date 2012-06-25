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

#include <cmath>

#include <ogl-opengl.hpp>

#include <etc-math.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-user.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "math3d.h"
#include "panoview.hpp"

//------------------------------------------------------------------------------

panoview::panoview(const std::string& exe,
                   const std::string& tag) : scm_viewer(exe, tag),
    min_zoom(0.5),
    max_zoom(4.0),
    drag_zooming(false),
    drag_looking(false)
{
    if (char *name = getenv("SCMINIT"))
         load(name);
}

panoview::~panoview()
{
}

//------------------------------------------------------------------------------

void panoview::draw(int frusi, const app::frustum *frusp, int chani)
{
    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (model)
    {
        double v[3];

        here.get_forward(v);

        model->set_zoom(v[0], v[1], v[2], here.get_zoom());
    }

    scm_viewer::draw(frusi, frusp, chani);
    scm_viewer::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

bool panoview::process_event(app::event *E)
{
    if (!scm_viewer::process_event(E))
    {
        switch (E->get_type())
        {
            case E_CLICK: return pan_click(E);
            case E_POINT: return pan_point(E);
            case E_TICK:  return pan_tick(E);
        }
    }
    return false;
}

//------------------------------------------------------------------------------

bool panoview::pan_point(app::event *E)
{
    if (const app::frustum *overlay = ::host->get_overlay())
        overlay->pointer_to_2D(E, curr_x, curr_y);

    return false;
}

bool panoview::pan_click(app::event *E)
{
    if (E->data.click.b == 0)
        drag_looking = E->data.click.d;
    if (E->data.click.b == 2)
        drag_zooming = E->data.click.d;

    drag_x = curr_x;
    drag_y = curr_y;

    return true;
}

bool panoview::pan_tick(app::event *E)
{
    double M[16];

    float dt = E->data.tick.dt / 1000.0;

    if (drag_zooming)
    {
        double z = here.get_zoom() + (curr_y - drag_y) / 500.0f;

        if (z < min_zoom) z = min_zoom;
        if (z > max_zoom) z = max_zoom;

        here.set_zoom(z);
    }
    if (drag_looking)
    {
        double y[3] = { 0.0, 1.0, 0.0 };
        double r[3];

        here.get_right(r);

        double dx = (double) (curr_x - drag_x) / 500.0;
        double dy = (double) (curr_y - drag_y) / 500.0;

        double X[16];
        double Y[16];

        mrotate(X, r,  10.0 * dy * dt);
        mrotate(Y, y, -10.0 * dx * dt);
        mmultiply(M, X, Y);

        here.transform_orientation(M);
    }

    here.get_matrix(M, get_scale(here.get_radius()));
    ::user->set_M(M);

    return false;
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        app::prog *P;

        P = new panoview(argv[0], std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
        P->run();

        delete P;
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "Exception: %s\n", e.what());
    }
    return 0;
}

//------------------------------------------------------------------------------
