//  Copyright (C) 2005 Robert Kooima
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

#include <sstream>

#include "main.hpp"
#include "control.hpp"

//-----------------------------------------------------------------------------

void cnt::editor::apply()
{
    scene.set_param(key, str);
}

void cnt::editor::show()
{
    int c;

    if ((c = scene.get_param(key, str)) == 0)
        str = "";

    update();

    is_enabled = (c > 0);
    is_varied  = (c > 1);
}

void cnt::editor::hide()
{
    if (is_varied == false) apply();
}

//-----------------------------------------------------------------------------

void cnt::bitmap::show()
{
}

void cnt::bitmap::hide()
{
}

//-----------------------------------------------------------------------------

void cnt::create_button::do_create(ent::entity *entity)
{
    // Select the new entity and add a create operation for it.

    scene.clear_selection();
    scene.click_selection(entity);
    scene.do_create();

    // Update the GUI to reflect the new entity state.

    state->show();
}

//-----------------------------------------------------------------------------

cnt::solid_panel::solid_panel(ops::scene& s, gui::widget *w) : gui::vgroup()
{
    gui::editor *E = new gui::editor("");
    gui::finder *F = new gui::finder("solid_directory", ".obj", E);

    add((new gui::frame)->
        add((new gui::hgroup)->
            add((new gui::vgroup)->

                add(new title("Create Solid"))->
                add(new new_box_button    (s, w, E))->
                add(new new_sphere_button (s, w, E))->
                add(new new_capsule_button(s, w, E))->
                add(new gui::filler(false, true)))->

            add((new gui::vgroup)->
                add((new gui::hgroup)->
                    add(new label("File"))->
                    add(E))->
                add(F))));

    add(new gui::spacer);

    add((new gui::frame)->
        add((new gui::harray)->
            add((new gui::hgroup)->
                add((new gui::varray)->

                    add(new label("Category"))->
                    add(new label("Collide"))->
                    add(new gui::filler(false, false)))->

                add((new gui::varray)->

                    add(new bitmap(s, ent::solid::category))->
                    add(new bitmap(s, ent::solid::collide))->
                    add(new gui::filler(false, false))))->

            add((new gui::hgroup)->
                add((new gui::varray)->

                    add(new label("Density"))->
                    add(new label("Bounce"))->
                    add(new label("Friction")))->

                add((new gui::varray)->

                    add(new editor(s, ent::solid::density))->
                    add(new editor(s, ent::solid::bounce))->
                    add(new editor(s, ent::solid::friction)))->

                add((new gui::varray)->

                    add(new label("Soft ERP"))->
                    add(new label("Soft CFM"))->
                    add(new gui::filler(false, false)))->

                add((new gui::varray)->

                    add(new editor(s, ent::solid::soft_erp))->
                    add(new editor(s, ent::solid::soft_cfm))->
                    add(new gui::filler(false, false))))));
}

//-----------------------------------------------------------------------------

cnt::world_panel::world_panel(ops::scene& s, gui::widget *w) : gui::vgroup()
{
    gui::editor *E = new gui::editor("");
    gui::finder *F = new gui::finder("world_directory", ".xml", E);

    add((new gui::frame)->
        add((new gui::hgroup)->
            add((new gui::vgroup)->

                add(new title("World"))->
                add(new init_button(s, w))->
                add(new load_button(s, w, E))->
                add(new save_all_button(s, w, E))->
                add(new save_sel_button(s, w, E))->
                add(new gui::filler(false, true)))->

            add((new gui::vgroup)->
                add((new gui::hgroup)->
                    add(new label("File"))->
                    add(E))->
                add(F))));
}

//-----------------------------------------------------------------------------

cnt::joint_panel::joint_panel(ops::scene& s, gui::widget *w) : gui::vgroup()
{
    add((new gui::frame)->
        add((new gui::harray)->

            add(new title("Create Joint"))->
            add(new new_ball_button     (s, w))->
            add(new new_hinge_button    (s, w))->
            add(new new_hinge2_button   (s, w))->
            add(new new_slider_button   (s, w))->
            add(new new_amotor_button   (s, w))->
            add(new new_universal_button(s, w))));

    add(new gui::spacer);

    add((new gui::frame)->
        add((new gui::harray)->

            add((new gui::varray)->
                add(new title("Configure Joint"))->

                add(new label("Velocity"))->
                add(new label("Force"))->
                add(new label("CFM"))->
                add(new label("Stop Bounce"))->
                add(new label("Lo Stop"))->
                add(new label("Hi Stop"))->
                add(new label("Stop ERP"))->
                add(new label("Stop CFM"))->
                add(new label("Suspension ERP"))->
                add(new label("Suspension CFM")))->

            add((new gui::varray)->
                add(new title("Axis 1"))->

                add(new editor(s, ent::joint::velocity))->
                add(new editor(s, ent::joint::force))->
                add(new editor(s, ent::joint::cfm))->
                add(new editor(s, ent::joint::bounce))->
                add(new editor(s, ent::joint::lo_stop))->
                add(new editor(s, ent::joint::hi_stop))->
                add(new editor(s, ent::joint::stop_erp))->
                add(new editor(s, ent::joint::stop_cfm))->
                add(new editor(s, ent::joint::susp_erp))->
                add(new editor(s, ent::joint::susp_cfm)))->

            add((new gui::varray)->
                add(new title("Axis 2"))->
                add(new editor(s, ent::joint::velocity2))->
                add(new editor(s, ent::joint::force2))->
                add(new editor(s, ent::joint::cfm2))->
                add(new editor(s, ent::joint::bounce2))->
                add(new editor(s, ent::joint::lo_stop2))->
                add(new editor(s, ent::joint::hi_stop2))->
                add(new editor(s, ent::joint::stop_erp2))->
                add(new editor(s, ent::joint::stop_cfm2))->

                add(new gui::filler)->
                add(new gui::filler))->

            add((new gui::varray)->
                add(new title("Axis 3"))->

                add(new editor(s, ent::joint::velocity3))->
                add(new editor(s, ent::joint::force3))->
                add(new editor(s, ent::joint::cfm3))->
                add(new editor(s, ent::joint::bounce3))->
                add(new editor(s, ent::joint::lo_stop3))->
                add(new editor(s, ent::joint::hi_stop3))->
                add(new editor(s, ent::joint::stop_erp3))->
                add(new editor(s, ent::joint::stop_cfm3))->

                add(new gui::filler)->
                add(new gui::filler))));
}

//-----------------------------------------------------------------------------

cnt::control::control(ops::scene& s)
{
    int w = conf->get_i("window_w");
    int h = conf->get_i("window_h");

    gui::option *O = new gui::option;

    root = ((new gui::vgroup)->
            add((new gui::harray)->
                add(new title("Control Panel", 0))->
                add(new panel_button("World", O, 0))->
                add(new panel_button("Solid", O, 1))->
                add(new panel_button("Joint", O, 2))->
                add(new gui::spacer))->
            add(new gui::spacer)->
            add(O->
                add(new world_panel(s, O))->
                add(new solid_panel(s, O))->
                add(new joint_panel(s, O))));

    root->layup();
    root->laydn((w - root->get_w()) / 2,
                (h - root->get_h()) / 2, root->get_w(), root->get_h());
}

//-----------------------------------------------------------------------------
