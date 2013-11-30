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

#include <etc-vector.hpp>
#include <app-data.hpp>
#include <app-host.hpp>
#include <app-view.hpp>
#include <app-conf.hpp>
#include <app-prog.hpp>
#include <app-event.hpp>
#include <app-frustum.hpp>
#include <app-default.hpp>

#include "scm/util3d/math3d.h"

#include "panoview.hpp"

//------------------------------------------------------------------------------

panoview::panoview(const std::string& exe,
                   const std::string& tag) :
    view_app(exe, tag),
    zoom(1.0),
    zoom_min(0.001),
    zoom_max(3.000)
{
    if (char *name = getenv("SCMINIT"))
    {
        load_file(name);

        if (sys && sys->get_step_count())
            move_to(0);
    }
}

panoview::~panoview()
{
}

//------------------------------------------------------------------------------

ogl::range panoview::prep(int frusc, const app::frustum *const *frusv)
{
    double p[3] = { 0.0, 0.0, 0.0      };
    double q[4] = { 0.0, 0.0, 0.0, 1.0 };
    double v[3] = { 0.0, 0.0, -1.0 };

#ifdef FIXME
    ::view->get_point(0, p, v, q);
#endif

    sys->get_sphere()->set_zoom(v[0], v[1], v[2], zoom);

    return view_app::prep(frusc, frusv);
}

void panoview::draw(int frusi, const app::frustum *frusp, int chani)
{
    view_app::draw(frusi, frusp, chani);
    view_app::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

bool panoview::process_event(app::event *E)
{
    if (E->get_type() == E_CLICK)
    {
         if (E->data.click.b == 4)
            zoom = std::max(zoom * 0.95, zoom_min);
         if (E->data.click.b == 5)
            zoom = std::min(zoom * 1.05, zoom_max);
    }
    return view_app::process_event(E);
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
