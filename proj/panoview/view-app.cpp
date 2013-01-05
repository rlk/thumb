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

#include "scm/util3d/math3d.h"

#include "view-app.hpp"

//------------------------------------------------------------------------------

view_app::view_app(const std::string& exe,
                   const std::string& tag) : app::prog(exe, tag),
    dtime(0),
    debug_cache(false),
    debug_label(false),
    debug_path (false)
{
    gui_init();

    // Cache the label font file.

    font_ptr = ::data->load(::conf->get_s("sans_font"), &font_len);

    // Create the SCM rendering system.

    TIFFSetWarningHandler(0);

    const int w = ::host->get_buffer_w();
    const int h = ::host->get_buffer_h();

    sys = new scm_system(w, h, 32, 512);
}

view_app::~view_app()
{
    delete sys;

    gui_free();

    ::data->free(::conf->get_s("sans_font"));
}

//------------------------------------------------------------------------------

void view_app::load_images(app::node p, scm_scene *f)
{
    // Create a new image object for each node.

    for (app::node i = p.find("image"); i; i = p.next(i, "image"))
    {
        if (scm_image *p = f->get_image(f->add_image(f->get_image_count())))
        {
            p->set_scm             (i.get_s("scm"));
            p->set_name            (i.get_s("name"));
            p->set_channel         (i.get_i("channel"));
            p->set_normal_min(float(i.get_f("k0", 0.0)));
            p->set_normal_max(float(i.get_f("k1", 1.0)));
        }
    }
}

void view_app::load_scenes(app::node p)
{
    // Create a new scene object for each node.

    for (app::node i = p.find("scene"); i; i = p.next(i, "scene"))
    {
        if (scm_scene *f = sys->get_scene(sys->add_scene(sys->get_scene_count())))
        {
            load_images(i, f);

            f->set_name (i.get_s("name"));
            f->set_label(i.get_s("label"));

            const std::string& vert_name = i.get_s("vert");
            const std::string& frag_name = i.get_s("frag");

            if (!vert_name.empty())
                f->set_vert((const char *) ::data->load(vert_name));
            if (!vert_name.empty())
                f->set_frag((const char *) ::data->load(frag_name));
        }
    }
}

void view_app::load_steps(app::node p)
{
    // Create a new step object for each node.

    for (app::node i = p.find("step"); i; i = p.next(i, "step"))
    {
        if (scm_step *s = sys->get_step(sys->add_step(sys->get_step_count())))
        {
            double q[4];
            double p[3];
            double l[3];

            q[0] = i.get_f("q0", 0.0);
            q[1] = i.get_f("q1", 0.0);
            q[2] = i.get_f("q2", 0.0);
            q[3] = i.get_f("q3", 1.0);

            p[0] = i.get_f("p0", 0.0);
            p[1] = i.get_f("p1", 0.0);
            p[2] = i.get_f("p2", 0.0);

            l[0] = i.get_f("l0", 0.0);
            l[1] = i.get_f("l1", 0.0);
            l[2] = i.get_f("l2", 0.0);

            s->set_name       (i.get_s("name"));
            s->set_scene      (i.get_s("scene"));
            s->set_orientation(q);
            s->set_position   (p);
            s->set_light      (l);
            s->set_speed      (i.get_f("s", 1.0));
            s->set_distance   (i.get_f("r", 0.0));
            s->set_tension    (i.get_f("t", 0.0));
            s->set_bias       (i.get_f("b", 0.0));
            s->set_zoom       (i.get_f("z", 1.0));

            sys->append_queue(s);
        }
    }
}

void view_app::load(const std::string& name)
{
    // If the named file exists and contains an XML panorama definition...

    app::file file(name);

    if (app::node root = file.get_root().find("sphere"))
    {
        // Load the new data.

        int scenes = sys->get_scene_count();
        int steps  = sys->get_step_count();

        load_scenes(root);
        load_steps (root);

        // Delete the old data.

        for (int i = 0; i < scenes; ++i) sys->del_scene(0);
        for (int i = 0; i < steps;  ++i) sys->del_step (0);

        sys->set_current_time (0);
        sys->set_current_scene(0);

        // Dismiss the GUI.

        gui_state = false;
    }
}

void view_app::cancel()
{
    gui_state = false;
}

void view_app::flag()
{
    double pos[3], rad = get_current_ground();

    here.get_position(pos);

    double lon = atan2(pos[0], pos[2]) * 180.0 / M_PI;
    double lat =  asin(pos[1])         * 180.0 / M_PI;

    printf("%.12f\t%.12f\t%.1f\n", lat, lon, rad);
}

//------------------------------------------------------------------------------

double view_app::get_current_ground() const
{
    double v[3];
    here.get_position(v);
    return sys->get_current_ground(v);
}

double view_app::get_minimum_ground() const
{
    return sys->get_minimum_ground();
}

//------------------------------------------------------------------------------

ogl::range view_app::prep(int frusc, const app::frustum *const *frusv)
{
    sys->update_cache(::host->get_movie_mode());

    return ogl::range(0.1, 2.0 * get_minimum_ground());
}

void view_app::lite(int frusc, const app::frustum *const *frusv)
{
}

void view_app::draw(int frusi, const app::frustum *frusp, int chani)
{
    const double *P =  frusp->get_P();
    const double *M = ::user->get_M();

    // Compute the model-view-projection matrix to be used for view culling.

    double T[16];
    double V[16];

    load_inv(V, M);
    mult_mat_mat(T, P, V);

    // Draw the sphere.

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(V);

    sys->render_sphere(T, chani);
}

void view_app::over(int frusi, const app::frustum *frusp, int chani)
{
    frusp->draw();
   ::user->draw();

    if (debug_path)  sys->render_queue();
    if (debug_cache) sys->render_cache();
    if (gui_state)   gui_draw();
}

//------------------------------------------------------------------------------

bool view_app::process_key(app::event *E)
{
    const int d = E->data.key.d;
    const int k = E->data.key.k;
    const int c = E->data.key.m & KMOD_CTRL;
    const int s = E->data.key.m & KMOD_SHIFT;

    scm_sphere *sph = sys->get_sphere();
    scm_render *ren = sys->get_render();

    if (d)
    {
        if (!c && !s)
            switch (k)
            {
                case '0': sys->set_current_scene(0); return true;
                case '1': sys->set_current_scene(1); return true;
                case '2': sys->set_current_scene(2); return true;
                case '3': sys->set_current_scene(3); return true;
                case '4': sys->set_current_scene(4); return true;
                case '5': sys->set_current_scene(5); return true;
                case '6': sys->set_current_scene(6); return true;
                case '7': sys->set_current_scene(7); return true;
                case '8': sys->set_current_scene(8); return true;
                case '9': sys->set_current_scene(9); return true;

                case 282: gui_state   = !gui_state;        return true; // F1
                case 283: debug_cache = !debug_cache;      return true; // F2
                case 284: debug_label = !debug_label;      return true; // F3
                case 285: debug_path  = !debug_path;       return true; // F4
                case 286: ren->set_wire(!ren->get_wire()); return true; // F5
                case 287:                                  return true; // F6
                case 288: sys->get_render()->set_blur( 0); return true; // F7
                case 289: sys->get_render()->set_blur(16); return true; // F8

                case 290: sph->set_detail(sph->get_detail() +  2); return true;
                case 291: sph->set_detail(sph->get_detail() -  2); return true;
                case 292: sph->set_limit (sph->get_limit () + 10); return true;
                case 293: sph->set_limit (sph->get_limit () - 10); return true;

                case 8: sys->flush_cache(); return true; // Backspace
            }

        if (c)
            switch (k)
            {
                case '0': make_path(0); return true;
                case '1': make_path(1); return true;
                case '2': make_path(2); return true;
                case '3': make_path(3); return true;
                case '4': make_path(4); return true;
                case '5': make_path(5); return true;
                case '6': make_path(6); return true;
                case '7': make_path(7); return true;
                case '8': make_path(8); return true;
                case '9': make_path(9); return true;
            }

        if (s)
            switch (k)
            {
                case '1': flag(); return true;
            }

        if (k == 32)
        {
            sys->set_current_time(0);
            if (dtime)
                dtime = 0;
            else
                dtime = 1;
            return true;
        }
    }

    return prog::process_event(E);
}

bool view_app::process_user(app::event *E)
{
    // Extract the landmark name from the user event structure.

    char name[sizeof (long long) + 1];

    memset(name, 0, sizeof (name));
    memcpy(name, &E->data.user.d, sizeof (long long));

    // Scan the steps for a matching name.

    for (int i = 0; i < sys->get_step_count(); i++)
    {
        if (sys->get_step(i)->get_name().compare(name) == 0)
        {
            make_path(i);
            return true;
        }
    }
    return false;
}

bool view_app::process_tick(app::event *E)
{
    double dt = E->data.tick.dt;

    if (dtime)
    {
        double ptime = sys->get_current_time();
        double ntime = ptime + dtime * here.get_speed() * dt;

        sys->set_current_time(ntime);

        if (ptime == sys->get_current_time())
            dtime = 0;
    }
    return false;
}

bool view_app::process_event(app::event *E)
{
    if      (E->get_type() == E_KEY)  return process_key(E);
    else if (E->get_type() == E_USER) return process_user(E);
    else if (E->get_type() == E_TICK) return process_tick(E);

    // Pass the event to the GUI if visible.

    else if (gui_state)
    {
        switch (E->get_type())
        {
            case E_CLICK: return gui_click(E);
            case E_POINT: return gui_point(E);
            case E_KEY:   return gui_key  (E);
        }
    }

    return false;
}

//------------------------------------------------------------------------------

void view_app::make_path(int i)
{
}

//------------------------------------------------------------------------------

// Initialize the file selection GUI.

void view_app::gui_init()
{
    const app::frustum *overlay = ::host->get_overlay();

    int w = overlay ? overlay->get_pixel_w() : ::host->get_buffer_w();
    int h = overlay ? overlay->get_pixel_h() : ::host->get_buffer_h();

    gui_state = true;

    ui = new view_load(this, w, h);
}

// Release the file selection GUI.

void view_app::gui_free()
{
    delete ui;
}

// Draw the file selection GUI in the overlay.

void view_app::gui_draw()
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        glEnable(GL_DEPTH_CLAMP_NV);
        {
            overlay->draw();
            overlay->overlay();
            ui->draw();
        }
        glDisable(GL_DEPTH_CLAMP_NV);
    }
}

// Handle a mouse point event while the GUI is visible.

bool view_app::gui_point(app::event *E)
{
    if (const app::frustum *overlay = ::host->get_overlay())
    {
        int x;
        int y;

        overlay->pointer_to_2D(E, x, y);
        ui->point(x, y);

        return true;
    }
    return false;
}

// Handle a mouse click event while the GUI is visible.

bool view_app::gui_click(app::event *E)
{
    ui->click(E->data.click.m,
              E->data.click.d);
    return true;
}

// Handle a keyboard event while the GUI is visible.

bool view_app::gui_key(app::event *E)
{
    if (E->data.key.d)
    {
        ui->key(E->data.key.c,
                E->data.key.k,
                E->data.key.m);
    }
    return true;
}

//------------------------------------------------------------------------------

