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
    zoom(0.0),
    zoom_min(-5.0),
    zoom_max( 2.0)
{
}

panoview::~panoview()
{
}

//------------------------------------------------------------------------------

ogl::range panoview::prep(int frusc, const app::frustum *const *frusv)
{
    vec3 v = ::view->get_point_vec(quat());

    sys->get_sphere()->set_zoom(v[0], v[1], v[2], pow(2.0, zoom));

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
        const int b = E->data.click.b;
        const int d = E->data.click.d;

        if (b == -2)
        {
            zoom = zoom + d / 100.0;
            zoom = std::max(zoom, zoom_min);
            zoom = std::min(zoom, zoom_max);
            return true;
        }
    }
    if (E->get_type() == E_KEY)
    {
        if (E->data.key.d && E->data.key.k == SDL_SCANCODE_SPACE)
        {
            ::view->go_home();
            zoom = 0.0;
            return true;
        }
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
