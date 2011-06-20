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

#include <app-host.hpp>
#include <app-user.hpp>
#include <app-frustum.hpp>

#include "math3d.h"
#include "panoview.hpp"

//-----------------------------------------------------------------------------

// sph-image
// sph-cache
// sph-model

panoview::panoview(const std::string& tag) : app::prog(tag)
{
}

panoview::~panoview()
{
}

//-----------------------------------------------------------------------------

ogl::range panoview::prep(int frusc, const app::frustum *const *frusv)
{
    return ogl::range(0.1, 10.0);
}

void panoview::lite(int frusc, const app::frustum *const *frusv)
{
}

void panoview::draw(int frusi, const app::frustum *frusp)
{
    const double *P =  frusp->get_P();
    const double *M = ::user->get_M();
    const int     w = ::host->get_buffer_w();
    const int     h = ::host->get_buffer_h();

    double V[16];

    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
 
    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    minvert(V, M);
    L.draw(P, V, w, h);
}

//-----------------------------------------------------------------------------