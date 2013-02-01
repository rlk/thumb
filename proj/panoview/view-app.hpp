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

#include "scm/scm-system.hpp"
#include "scm/scm-sphere.hpp"
#include "scm/scm-render.hpp"
#include "scm/scm-image.hpp"
#include "scm/scm-label.hpp"
#include "scm/scm-step.hpp"

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

    virtual void   load_file(const std::string&);
    virtual void   load_path(const std::string&);
    virtual void unload();
    virtual void reload();

    void cancel();
    void flag();

    double get_current_ground() const;
    double get_minimum_ground() const;

    virtual void move_to(int) { }
    virtual void fade_to(int) { }

protected:

    scm_system *sys;
    scm_step   path_src;
    scm_step   path_mid;
    scm_step   path_dst;
    scm_step   here;
    double     now;
    double     delta;

private:

    void save_steps (app::node);
    void load_steps (app::node);
    void load_images(app::node, scm_scene *);
    void load_scenes(app::node);

    bool draw_cache;
    bool draw_path;
    bool draw_gui;

    bool numkey(int, int, int);
    bool funkey(int, int, int);

    bool process_key (app::event *);
    bool process_user(app::event *);
    bool process_tick(app::event *);

    // Path editing handlers

    const std::string path_xml;

    void set_step(int);
    int  step;

    void path_save();
    void path_load();
    void path_queue();
    void path_clear();
    void path_play(bool);
    void path_prev();
    void path_next();
    void path_del();
    void path_ins();
    void path_add();
    void path_set();
    void path_beg();
    void path_end();
    void path_jump();

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
