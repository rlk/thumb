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
    draw_cache(false),
    draw_path (false),
    step(0)
{
    gui_init();

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

        draw_gui = false;
    }
}

void view_app::cancel()
{
    draw_gui = false;
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

    if (draw_path)
    {
        if (scm_step *s = sys->get_step(step))
        {
            glPushAttrib(GL_ENABLE_BIT);
            glEnable(GL_DEPTH_CLAMP);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glPointSize(12.0);
            glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
            glBegin(GL_POINTS);
            s->draw();
            glEnd();
            glPopAttrib();
        }
    }

    if (draw_path)  sys->render_queue();
    if (draw_cache) sys->render_cache();

    if (draw_gui)   gui_draw();
}

//------------------------------------------------------------------------------

void view_app::set_step(int s)
{
    step = s;
    step = std::max(step, 0);
    step = std::min(step, sys->get_step_count() - 1);
}

void view_app::path_queue()
{
    sys->flush_queue();

    for (int i = 0; i < sys->get_step_count(); i++)
        sys->append_queue(sys->get_step(i));
}

void view_app::path_clear()
{
    step  = 0;
    dtime = 0;
    while (sys->get_step_count())
        sys->del_step(0);
    path_queue();
}

void view_app::path_play()
{
    if (dtime > 0)
        dtime = 0;
    else
        dtime = 1;
    path_queue();
}

void view_app::path_prev()
{
    set_step(step - 1);
    path_queue();
}

void view_app::path_next()
{
    set_step(step + 1);
    path_queue();
}

void view_app::path_save()
{
}

void view_app::path_load()
{
}

void view_app::path_del()
{
    sys->del_step(step);
    set_step(step - 1);
    path_queue();
}

void view_app::path_ins()
{
    if (scm_step *s = sys->get_step(sys->add_step(step)))
    {
        *s = here;
        path_queue();
    }
}

void view_app::path_add()
{
    if (scm_step *s = sys->get_step(sys->add_step(step + 1)))
    {
        *s = here;
        set_step(step + 1);
        path_queue();
    }
}

void view_app::path_set()
{
    if (scm_step *s = sys->get_step(step))
    {
        *s = here;
        path_queue();
    }
}

void view_app::path_jump()
{
    if (scm_step *s = sys->get_step(step))
    {
        here = *s;
        sys->set_current_time(step);
    }
}

void view_app::path_beg()
{
    set_step(0);
    path_queue();
}

void view_app::path_end()
{
    set_step(sys->get_step_count());
    path_queue();
}

//------------------------------------------------------------------------------

bool view_app::numkey(int n, int c, int s)
{
    if (s == 0)
    {
        if (c == 0)
            move_to(n);
        else
            fade_to(n);
    }
    else
    {
        switch (n)
        {
            case 1: flag(); break;
        }
    }
    return true;
}

bool view_app::funkey(int n, int c, int s)
{
    if (s == 0)
    {
        if (c == 0)
        {
            scm_sphere *sph = sys->get_sphere();
            scm_render *ren = sys->get_render();

            switch (n)
            {
                case  1: draw_gui   = !draw_gui;                  break;
                case  2: draw_cache = !draw_cache;                break;
                case  3: draw_path  = !draw_path;                 break;
                case  4:                                          break;

                case  5: ren->set_wire(!ren->get_wire());         break;
                case  6:                                          break;
                case  7: ren->set_blur( 0);                       break;
                case  8: ren->set_blur(16);                       break;

                case  9: sph->set_detail(sph->get_detail() +  2); break;
                case 10: sph->set_detail(sph->get_detail() -  2); break;
                case 11: sph->set_limit (sph->get_limit () + 10); break;
                case 12: sph->set_limit (sph->get_limit () - 10); break;
            }
        }
    }

    return true;
}

bool view_app::process_key(app::event *E)
{
    const int d = E->data.key.d;
    const int k = E->data.key.k;
    const int c = E->data.key.m & KMOD_CTRL;
    const int s = E->data.key.m & KMOD_SHIFT;

    if (d)
    {
        if ('0' <= k && k <= '9') return numkey(k - '0', c, s);
        if (282 <= k && k <= 293) return funkey(k - 281, c, s);

        if (c)
        {
#if 1
            switch (k)
            {
                case 'c': path_clear(); return true; // ^C
                case 'p': path_prev();  return true; // ^P
                case 'n': path_next();  return true; // ^N
                case 's': path_save();  return true; // ^S
                case 'l': path_load();  return true; // ^L
                case 'd': path_del();   return true; // ^D
                case 'i': path_ins();   return true; // ^I
                case 'a': path_add();   return true; // ^A
                case 'o': path_set();   return true; // ^O
                case 'b': path_beg();   return true; // ^B
                case 'e': path_end();   return true; // ^E
                case 'j': path_jump();  return true; // ^J
            }
#else
            switch (k)
            {
                case 'c': path.clear();    return true; // ^C
                case 'b': path.back(s);    return true; // ^B
                case 'f': path.fore(s);    return true; // ^F
                case 'p': path.prev();     return true; // ^P
                case 'n': path.next();     return true; // ^N
                case 's': path.save();     return true; // ^S
                case 'l': path.load();     return true; // ^L
                case 'd': path.del();      return true; // ^D
                case 'i': path.ins(here);  return true; // ^I
                case 'a': path.add(here);  return true; // ^A
                case 'o': path.set(here);  return true; // ^O
                case 'r': path.home();     return true; // ^R
                case 'j': path.jump();
                          path.get(here);  return true; // ^J
            }
#endif
        }
        else
        {
            if (k == 8)
            {
                sys->flush_cache();
                return true;
            }
            if (k == 32)
            {
                path_play();
                return true;
            }
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
            move_to(i);
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
    // Delegate keypresses, user commands, and the passage of time.

    if      (E->get_type() == E_KEY)  return process_key(E);
    else if (E->get_type() == E_USER) return process_user(E);
    else if (E->get_type() == E_TICK) return process_tick(E);

    // Pass the event to the GUI if visible.

    else if (draw_gui)
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

// Initialize the file selection GUI.

void view_app::gui_init()
{
    const app::frustum *overlay = ::host->get_overlay();

    int w = overlay ? overlay->get_pixel_w() : ::host->get_buffer_w();
    int h = overlay ? overlay->get_pixel_h() : ::host->get_buffer_h();

    draw_gui = true;

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
        glEnable(GL_DEPTH_CLAMP);
        {
            overlay->draw();
            overlay->overlay();
            ui->draw();
        }
        glDisable(GL_DEPTH_CLAMP);
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
