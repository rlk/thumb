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

#ifndef VIEW_APP_HPP
#define VIEW_APP_HPP

#include <vector>

#include <app-prog.hpp>
#include <app-file.hpp>

#include "scm-cache.hpp"
#include "scm-frame.hpp"
#include "scm-model.hpp"
#include "scm-label.hpp"
#include "view-step.hpp"
#include "view-path.hpp"
#include "view-load.hpp"

//-----------------------------------------------------------------------------

class view_app : public app::prog
{
public:

    view_app(const std::string&, const std::string&);
   ~view_app();

    virtual ogl::range prep(int, const app::frustum * const *);
    virtual void       lite(int, const app::frustum * const *);
    virtual void       draw(int, const app::frustum *, int);
    virtual void       over(int, const app::frustum *, int);

    virtual bool process_event(app::event *);

    virtual void load(const std::string&);

    void unload();
    void cancel();

    void goto_next();
    void goto_prev();

    double get_radius() const   { return radius; }
    void   set_radius(double r) { radius = r;    }

    virtual double get_scale(double) const { return 1.0; }
    virtual void   make_path(int);

protected:

    scm_cache_v caches;
    scm_frame_v frames;

    scm_cache *bound;
    scm_model *model;
    scm_label *label;

    view_step here;
    view_path path;

    std::vector<view_step> steps;

    bool gui_state;

private:

    void load_model (app::node);
    void load_caches(app::node);
    void load_images(app::node, scm_frame *);
    void load_frames(app::node);
    void load_steps (app::node);

    // Sphere rendering state

    double timer;
    double timer_d;
    double timer_e;
    double height;
    double radius;

    bool debug_cache;
    bool debug_label;
    bool debug_path;
    bool debug_wire;
    bool debug_bound;

    bool process_key (app::event *);
    bool process_user(app::event *);
    bool process_tick(app::event *);

    // Label data

    const void *font_ptr;
    size_t      font_len;

    void load_label(const std::string&);

    // Sphere GUI State

    view_load *ui;

    void gui_init();
    void gui_free();
    void gui_draw();
    bool gui_point(app::event *);
    bool gui_click(app::event *);
    bool gui_key  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
