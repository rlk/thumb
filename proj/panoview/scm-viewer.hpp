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

#ifndef SCM_VIEWER_HPP
#define SCM_VIEWER_HPP

#include <vector>

#include <app-prog.hpp>
#include <app-file.hpp>

#include "scm-step.hpp"
#include "scm-path.hpp"
#include "scm-cache.hpp"
#include "scm-model.hpp"
#include "scm-label.hpp"
#include "scm-loader.hpp"

//-----------------------------------------------------------------------------

class scm_frame
{
    bool mono;

    struct image
    {
        int file;
        int shader;
        int channel;
    };

public:

    scm_frame(scm_cache *, app::node);

    int file(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].file;
    }
    int shader(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].shader;
    }
    int channel(int i)
    {
        return images.empty() ? 0 : images[i % images.size()].channel;
    }
    int size()
    {
        return images.size();
    }
    void apply(int c, std::vector<int>& tovert,
                      std::vector<int>& tofrag)
    {
        std::vector<image>::iterator i;

        for (i = images.begin(); i != images.end(); ++i)
            if (mono || c == i->channel)
            {
                if (i->shader)
                    tofrag.push_back(i->file);
                else
                    tovert.push_back(i->file);
            }
    }

private:

    std::vector<image> images;
};

//-----------------------------------------------------------------------------

class scm_viewer : public app::prog
{
public:

    scm_viewer(const std::string&, const std::string&);
   ~scm_viewer();

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

protected:

    scm_cache *cache;
    scm_model *model;
    scm_label *label;

    scm_step here;
    scm_path path;

    bool gui_state;

private:

    // Sphere rendering state

    std::vector<scm_frame *> frame;
    std::vector<int> tovert;
    std::vector<int> tofrag;
    std::vector<int> toprep;

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
    bool process_tick(app::event *);

    // Label data

    size_t      data_len;
    size_t      font_len;
    const void *data_ptr;
    const void *font_ptr;

    // Sphere GUI State

    scm_loader *ui;

    void gui_init();
    void gui_free();
    void gui_draw();
    bool gui_point(app::event *);
    bool gui_click(app::event *);
    bool gui_key  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
