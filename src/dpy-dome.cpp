//  Copyright (C) 2007 Robert Kooima
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

#include "dpy-dome.hpp"
#include "matrix.hpp"
#include "app-glob.hpp"
#include "app-user.hpp"
#include "app-prog.hpp"

//-----------------------------------------------------------------------------

dpy::dome::dome(app::node tile, app::node node, const int *window)
    : proj_frust(0), user_frust(0), draw_P(0), test_P(0)
{
    app::node curr;

    x = window[0];
    y = window[1];
    w = window[2];
    h = window[3];

    // Check the tile definition for the projector frustum.

    if ((curr = app::find(tile, "frustum")))
        proj_frust = new app::frustum(curr, w, h);
    else
        proj_frust = new app::frustum(   0, w, h);

    // Check the display definition for the user frustum.

    if ((curr = app::find(node, "frustum")))
        user_frust = new app::frustum(curr, w, h);
    else
        user_frust = new app::frustum(   0, w, h);

    // Note the view index.

    index = app::get_attr_d(node, "view");

    // Get other stuff.

    radius = app::get_attr_f(node, "radius", 27.5);

    tk[0] = app::get_attr_f(node, "t0",   0.0);
    tk[1] = app::get_attr_f(node, "t1",  90.0);
    tk[2] = app::get_attr_f(node, "dt",  10.0);

    pk[0] = app::get_attr_f(node, "p0",   0.0);
    pk[1] = app::get_attr_f(node, "p1",  90.0);
    pk[2] = app::get_attr_f(node, "dp",  10.0);

    zk[0] = app::get_attr_f(node, "z0", -10.0);
    zk[1] = app::get_attr_f(node, "z1",  10.0);
    zk[2] = app::get_attr_f(node, "dz",   0.25);
}

dpy::dome::~dome()
{
    if (user_frust) delete user_frust;
    if (proj_frust) delete proj_frust;
}

//-----------------------------------------------------------------------------

bool dpy::dome::input_point(int i, const double *p, const double *q)
{
    return proj_frust ? proj_frust->input_point(i, p, q) : false;
}

bool dpy::dome::input_click(int i, int b, int m, bool d)
{
    return proj_frust ? proj_frust->input_click(i, b, m, d) : false;
}

bool dpy::dome::input_keybd(int c, int k, int m, bool d)
{
    return proj_frust ? proj_frust->input_keybd(c, k, m, d) : false;
}

//-----------------------------------------------------------------------------

bool dpy::dome::pick(double *p, double *q, int wx, int wy)
{
    if (user_frust)
    {
        double kx = double(wx - x) / w;
        double ky = double(wy - y) / h;

        if (0.0 <= kx && kx < 1.0 &&
            0.0 <= ky && ky < 1.0)
            user_frust->pick(p, q, kx, ky);

        return true;
    }
    return false;
}

void dpy::dome::prep(app::view_v& views, app::frustum_v& frusta)
{
    double p[3] = { 0.0, 0.0, 0.0 };

    if (proj_frust)
        proj_frust->calc_user_planes(p);

    if (user_frust)
    {
        // Apply the viewpoint and view to my frustum.

        user_frust->calc_user_planes(views[index]->get_p());
        user_frust->calc_view_planes(::user->get_M(),
                                     ::user->get_I());
        user_frust->calc_projection(1.0, 2.0 * radius);

        // Add my frustum to the list.

        frusta.push_back(user_frust);
    }

    // Ensure the shaders are initialized.

    if ((draw_P == 0) &&
        (draw_P = ::glob->load_program("glsl/dpy/dome-draw.vert",
                                       "glsl/dpy/dome-draw.frag")))
    {
        draw_P->bind();
        {
            draw_P->uniform("map", 0);
        }
        draw_P->free();
    }
    if ((test_P == 0) &&
        (test_P = ::glob->load_program("glsl/dpy/dome-test.vert",
                                       "glsl/dpy/dome-test.frag")))
    {
    }
}

void dpy::dome::draw(app::view_v& views, int &i, bool calibrate, bool me)
{
    if (views[index])
    {
        // Draw the scene to the off-screen buffer.

        views[index]->bind();
        {
            ::prog->draw(i++);
        }
        views[index]->free();

        // Draw the off-screen buffer to the screen.

        glViewport(x, y, w, h);

        if (calibrate)
        {
            if (test_P)
            {
                test_P->bind();
                {
                    const double *p = proj_frust->get_user_pos();

                    test_P->uniform("pos", p[0], p[1], p[2]);
                    test_P->uniform("rad", radius);

                    test_P->uniform("tk", RAD(tk[0]), RAD(tk[1]), RAD(tk[2]));
                    test_P->uniform("pk", RAD(pk[0]), RAD(pk[1]), RAD(pk[2]));
                    test_P->uniform("zk",     zk[0],      zk[1],      zk[2]);

                    proj_frust->cast();
                }
                test_P->free();
            }
        }
        else
        {
            if (draw_P)
            {
                views[index]->bind_color(GL_TEXTURE0);
                draw_P->bind();
                {
                    const double *p = proj_frust->get_user_pos();

                    draw_P->uniform("pos", p[0], p[1], p[2]);
                    draw_P->uniform("rad", radius);
                    draw_P->uniform("siz", views[index]->get_w(),
                                           views[index]->get_h());

                    draw_P->uniform("tk", RAD(tk[0]), RAD(tk[1]), RAD(tk[2]));
                    draw_P->uniform("pk", RAD(pk[0]), RAD(pk[1]), RAD(pk[2]));
                    draw_P->uniform("zk",     zk[0],      zk[1],      zk[2]);

                    draw_P->uniform("proj", user_frust->get_P());

                    proj_frust->cast();
                }
                draw_P->free();
                views[index]->free_color(GL_TEXTURE0);
            }
        }
    }
}

//-----------------------------------------------------------------------------
