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
#include "orbiter.hpp"

//------------------------------------------------------------------------------

orbiter::orbiter(const std::string& exe,
                 const std::string& tag) : scm_viewer(exe, tag)
{
    drag_move = false;
    drag_look = false;
    drag_dive = false;
    drag_lite = false;

    current.set_radius(2000000.0);
    current.set_scale (1.0 / 1737400.0);
}

orbiter::~orbiter()
{
}

//------------------------------------------------------------------------------

// Apply the view panning interaction, rotating left-right about the position
// vector and up-down about the current right vector.

void orbiter::look(const double *point,
                   const double *click, double dt, double k)
{
    double r[3];
    double p[3];

    current.get_right(r);
    current.get_position(p);

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];
    double X[16];
    double Y[16];
    double M[16];

    mrotate(X, r,  10.0 * dy * dt);
    mrotate(Y, p, -10.0 * dx * dt);
    mmultiply(M, X, Y);

    current.transform_orientation(M);
}

// Apply the orbital motion interaction, using the direction of the mouse
// pointer motion to set the orbital plane and the magnitude of the mouse
// motion to set the orbital speed.

void orbiter::move(const double *point,
                   const double *click, double dt, double k)
{
    if (click[0] != point[0] ||
        click[1] != point[1] ||
        click[2] != point[2])
    {
        double d = vdot(click, point);
        double a = acos(d);

        if (fabs(a) > 0.0)
        {
            const double *M = ::user->get_M();
            double t[3];
            double u[3];

            vcrs(t, click, point);

            vtransform(u, M, t);
            vnormalize(orbit_plane, u);
        }
        orbit_speed = 10.0 * a * k;
    }
    else
    {
        orbit_speed = 0.0;
    }
}

// Apply the scale interaction, using the change in the mouse pointer position
// to affect an exponential difference in the scale of (and therefore apparent
// proximity to) the sphere. This causes the stereoscopic disparity to change
// in harmony with the altitude of the view.

void orbiter::dive(const double *point,
                   const double *click, double dt, double k)
{
    double d = (point[1] - click[1]) * dt;
    double r = current.get_radius();
    double m = get_radius() * (cache ? cache->get_r0() : 1.0);

    r = m + exp(log(r - m) + (4 * d));

    current.set_radius(r);
    current.set_scale(1.0 / (r - m));
}

// Apply the light position interaction, using the change in the mouse pointer
// to move the light source position vector. Light source position is relative
// to the camera, not to the sphere.

void orbiter::lite(const double *point,
                   const double *click, double dt, double k)
{
    double r[3];
    double u[3];

    current.get_right(r);
    current.get_up(u);

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];
    double X[16];
    double Y[16];
    double M[16];

    mrotate(X, r,  10.0 * dy * dt);
    mrotate(Y, u, -10.0 * dx * dt);
    mmultiply(M, X, Y);

    current.transform_light(M);
}

//------------------------------------------------------------------------------

ogl::range orbiter::prep(int frusc, const app::frustum *const *frusv)
{
    if (cache)
    {
        // Cycle the cache.

        cache->update(model->tick());

        // Compute a horizon line based upon altitude and  minimum data radius.

        double r = current.get_scale() *         get_radius() * cache->get_r0();
        double a = current.get_scale() * current.get_radius() + r;

        double n = 0.001 *     (a     - r    );
        double f = 1.1   * sqrt(a * a - r * r);

        return ogl::range(n, f);
    }
    return ogl::range(0.1, 10.0);
}

void orbiter::draw(int frusi, const app::frustum *frusp, int chani)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double  l[3];
    GLfloat L[4];

    current.get_light(l);

    L[0] = GLfloat(l[0]);
    L[1] = GLfloat(l[1]);
    L[2] = GLfloat(l[2]);
    L[3] = 0.0f;

    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, L);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glFrontFace(GL_CW);
    scm_viewer::draw(frusi, frusp, chani);

    glFrontFace(GL_CCW);
    scm_viewer::over(frusi, frusp, chani);
}

//------------------------------------------------------------------------------

bool orbiter::process_event(app::event *E)
{
    if (!scm_viewer::process_event(E))
    {
        switch (E->get_type())
        {
            case E_CLICK: return pan_click(E);
            case E_POINT: return pan_point(E);
            case E_TICK:  return pan_tick(E);
            case E_KEY:   return pan_key(E);
        }
    }

    return false;
}

void orbiter::load(const std::string& name)
{
    scm_viewer::load(name);
}

//------------------------------------------------------------------------------

bool orbiter::pan_point(app::event *E)
{
    double M[16];

    // Determine the point direction of the given quaternion.

    quat_to_mat(M, E->data.point.q);

    point[0] = -M[ 8];
    point[1] = -M[ 9];
    point[2] = -M[10];

    return false;
}

bool orbiter::pan_click(app::event *E)
{
    const int  b = E->data.click.b;
    const int  m = E->data.click.m;
    const bool d = E->data.click.d;
    const bool s = (m & KMOD_SHIFT);
    const bool c = (m & KMOD_CTRL);

    control = c;

    vcpy(click, point);

    if (d)
    {
        if (b == 0)
        {
            if      (c)
                drag_turn = true;
            else if (s)
                drag_look = true;
            else
                drag_move = true;
        }
        if (b == 2)
        {
            if (s)
                drag_lite = true;
            else
                drag_dive = true;
        }
    }
    else
    {
        if (b == 0) drag_look = drag_turn = drag_move = false;
        if (b == 2) drag_lite = drag_dive             = false;
    }

    return true;
}

bool orbiter::pan_tick(app::event *E)
{
    double dt = E->data.tick.dt / 1000.0;
    double sc = 1.0 / (get_radius() * current.get_scale());

    double M[16];

    if (control) sc *= 0.1;

    if (drag_move) move(point, click, dt, sc);
    if (drag_look) look(point, click, dt, sc);
    if (drag_dive) dive(point, click, dt, sc);
    if (drag_lite) lite(point, click, dt, sc);

    // Move the position and view orientation along the current orbit.

    if (orbit_speed > 0.0)
    {
        double R[16];

        mrotate(R, orbit_plane, orbit_speed * dt);

        current.transform_orientation(R);
        current.transform_position(R);
        current.transform_light(R);
    }

    current.get_matrix(M);
    ::user->set_M(M);

    return false;
}

bool orbiter::pan_key(app::event *E)
{
    const int d = E->data.key.d;
    const int k = E->data.key.k;
//  const int c = E->data.key.m & KMOD_CTRL;
//  const int s = E->data.key.m & KMOD_SHIFT;

    if (d)
    {
        switch (k)
        {
        case 280: goto_next(); return true;
        case 281: goto_prev(); return true;
        }
    }
    return false;
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        app::prog *P;

        P = new orbiter(argv[0], std::string(argc > 1 ? argv[1] : DEFAULT_TAG));
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
