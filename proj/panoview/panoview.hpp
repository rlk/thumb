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

#ifndef PANOVIEW_HPP
#define PANOVIEW_HPP

#include <vector>

#include <app-prog.hpp>

#include "sph-cache.hpp"
#include "sph-model.hpp"
#include "gui-loader.hpp"

//-----------------------------------------------------------------------------

class panoview : public app::prog
{
public:

    panoview(const std::string&);
   ~panoview();

    ogl::range prep(int, const app::frustum * const *);
    void       lite(int, const app::frustum * const *);
    void       draw(int, const app::frustum *);
    
    virtual bool process_event(app::event *);
    
    void load(const std::string&);
    void cancel();
    
private:

    // Rendering state

    std::vector<int> pan;
    
    sph_cache *C;
    sph_model *L;
    
    bool debug_zoom;
    bool debug_cache;
    bool debug_color;

    int spin;

    // GUI State

    loader *gui;
    
    bool gui_state;
    void gui_init();
    void gui_free();
    void gui_draw();
    bool gui_point(app::event *);
    bool gui_click(app::event *);
    bool gui_key  (app::event *);

    // Interaction state
    
    bool   drag_state;
    int    drag_x;
    int    drag_y;
    double drag_zoom;
    int    curr_x;
    int    curr_y;
    double curr_zoom;

    bool pan_point(app::event *);
    bool pan_click(app::event *);
    bool pan_key  (app::event *);
};

//-----------------------------------------------------------------------------

#endif
