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
}

orbiter::~orbiter()
{
}

//------------------------------------------------------------------------------

orbiter::state::state()
{
    distance       = 2.0;
    scale          = 1.0 / 1737400.0;

    orbit_plane[0] = 0.0;
    orbit_plane[1] = 1.0;
    orbit_plane[2] = 0.0;
    orbit_speed    = 0.0;

    position[0]    = 0.0;
    position[1]    = 0.0;
    position[2]    = 1.0;

    view_x[0]      = 1.0;
    view_x[1]      = 0.0;
    view_x[2]      = 0.0;

    view_y[0]      = 0.0;
    view_y[1]      = 1.0;
    view_y[2]      = 0.0;

    light[0]       = sqrt(3.0);
    light[1]       = sqrt(3.0);
    light[2]       = sqrt(3.0);
}

// Apply the view panning interaction, rotating left-right about the position
// vector and up-down about the current right vector.

void orbiter::state::look(const double *point,
                          const double *click, double dt, double k)
{
    const double *M = ::user->get_M();

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];

    double X[16];
    double Y[16];
    double R[16];
    double t[3];

    mrotate(X, M,         10.0 * dy * dt);
    mrotate(Y, position, -10.0 * dx * dt);
    mmultiply(R, X, Y);

    vtransform(t, R, view_x);
    vnormalize(view_x, t);

    vtransform(t, R, view_y);
    vnormalize(view_y, t);
}

// Apply the

void orbiter::state::turn(const double *point,
                          const double *click, double dt, double k)
{
    const double *M = ::user->get_M();

    double dx = point[0] - click[0];
    double dy = point[1] - click[1];

    double X[16];
    double Y[16];
    double R[16];
    double t[3];

    mrotate(X, M,         10.0 * dy * dt);
    mrotate(Y, position, -10.0 * dx * dt);
    mmultiply(R, X, Y);

    vtransform(t, R, view_x);
    vnormalize(view_x, t);

    vtransform(t, R, view_y);
    vnormalize(view_y, t);

    vtransform(t, Y, orbit_plane);
    vnormalize(orbit_plane, t);
}

// Apply the orbital motion interaction, using the direction of the mouse
// pointer motion to set the orbital plane and the magnitude of the mouse
// motion to set the orbital speed.

void orbiter::state::move(const double *point,
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

void orbiter::state::dive(const double *point,
                          const double *click, double dt, double k)
{
    double d = (point[1] - click[1]) * dt;

    scale = exp(log(scale) - 4.0 * d);
}

// Apply the light position interaction, using the change in the mouse pointer
// to move the light source position vector. Light source position is relative
// to the camera, not to the sphere.

void orbiter::state::lite(const double *point,
                          const double *click, double dt, double k)
{
    double dx = point[0] - click[0];
    double dy = point[1] - click[1];

    double X[16];
    double Y[16];
    double R[16];
    double t[3];

    mrotate(X, view_x, -10.0 * dy * dt);
    mrotate(Y, view_y,  10.0 * dx * dt);
    mmultiply(R, X, Y);

    vtransform(t, R, light);
    vnormalize(light, t);
}

// Animate all attributes of the view state over time dt, giving matrix M.

void orbiter::state::update(double dt, double *M, double r)
{
    // Move the position and view orientation along the current orbit.

    double t[3];
    double R[16];

    mrotate(R, orbit_plane, orbit_speed * dt);

    vtransform(t, R, view_x);
    vnormalize(view_x, t);

    vtransform(t, R, view_y);
    vnormalize(view_y, t);

    vtransform(t, R, light);
    vnormalize(light, t);

    vtransform(t, R, position);
    vnormalize(position, t);

    // Orthonormalize the view orientation basis.

    double view_z[3];

    vcrs(view_z, view_x, view_y);
    vcrs(view_y, view_z, view_x);

    mbasis(M, view_x, view_y, view_z);

    // Update the user transform.

    M[14] = position[2] * (r * scale + distance);
    M[12] = position[0] * (r * scale + distance);
    M[13] = position[1] * (r * scale + distance);
}

//------------------------------------------------------------------------------

#define FILENAME "orbiter.dat"

void orbiter::loadstate()
{
    FILE *stream;

    if ((stream = fopen(FILENAME, "r")))
    {
        fread(saved, sizeof (saved), 1, stream);
        fclose(stream);
    }
}

void orbiter::savestate()
{
    FILE *stream;

    if ((stream = fopen(FILENAME, "wb")))
    {
        fwrite(saved, sizeof (saved), 1, stream);
        fclose(stream);
    }
}

//------------------------------------------------------------------------------

ogl::range orbiter::prep(int frusc, const app::frustum *const *frusv)
{
    if (cache && model)
    {
        cache->update(model->tick());

        double r = current.scale * get_radius() * cache->get_r0();
        double a = current.scale * get_radius() * cache->get_r0() + current.distance;

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

    GLfloat L[4];

    L[0] = GLfloat(current.light[0]);
    L[1] = GLfloat(current.light[1]);
    L[2] = GLfloat(current.light[2]);
    L[3] = 0.0f;

    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, L);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    const double r = get_radius();

    set_radius(r * current.scale);
    {
        glFrontFace(GL_CW);
        scm_viewer::draw(frusi, frusp, chani);

        glFrontFace(GL_CCW);
        scm_viewer::over(frusi, frusp, chani);
    }
    set_radius(r);
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
    double sc = 1.0 / (get_radius() * current.scale);
    double r0 = cache ? cache->get_r0() : 1.0;

    double M[16];

    if (control) sc *= 0.1;

    if (drag_move) current.move(point, click, dt, sc);
    if (drag_look) current.look(point, click, dt, sc);
    if (drag_turn) current.turn(point, click, dt, sc);
    if (drag_dive) current.dive(point, click, dt, sc);
    if (drag_lite) current.lite(point, click, dt, sc);

    current.update(dt, M, get_radius() * r0);

    ::user->set_M(M);

    return false;
}

bool orbiter::pan_key(app::event *E)
{
    const int d = E->data.key.d;
    const int k = E->data.key.k;
    const int c = E->data.key.m & KMOD_CTRL;
    const int s = E->data.key.m & KMOD_SHIFT;

    if (d)
    {
        if (c)
            switch (k)
            {
                case 282: loadstate(); current = saved[ 0]; return true;
                case 283: loadstate(); current = saved[ 1]; return true;
                case 284: loadstate(); current = saved[ 2]; return true;
                case 285: loadstate(); current = saved[ 3]; return true;
                case 286: loadstate(); current = saved[ 4]; return true;
                case 287: loadstate(); current = saved[ 5]; return true;
                case 288: loadstate(); current = saved[ 6]; return true;
                case 289: loadstate(); current = saved[ 7]; return true;
                case 290: loadstate(); current = saved[ 8]; return true;
                case 291: loadstate(); current = saved[ 9]; return true;
                case 292: loadstate(); current = saved[10]; return true;
                case 293: loadstate(); current = saved[11]; return true;
            }
        if (s)
            switch (k)
            {
                case 282: saved[ 0] = current; savestate(); return true;
                case 283: saved[ 1] = current; savestate(); return true;
                case 284: saved[ 2] = current; savestate(); return true;
                case 285: saved[ 3] = current; savestate(); return true;
                case 286: saved[ 4] = current; savestate(); return true;
                case 287: saved[ 5] = current; savestate(); return true;
                case 288: saved[ 6] = current; savestate(); return true;
                case 289: saved[ 7] = current; savestate(); return true;
                case 290: saved[ 8] = current; savestate(); return true;
                case 291: saved[ 9] = current; savestate(); return true;
                case 292: saved[10] = current; savestate(); return true;
                case 293: saved[11] = current; savestate(); return true;
            }
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
