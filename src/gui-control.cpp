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

#include <cassert>
#include <sstream>

#include <gui-control.hpp>
#include <wrl-joint.hpp>
#include <wrl-solid.hpp>
#include <wrl-light.hpp>
#include <app-data.hpp>
#include <app-host.hpp>

//-----------------------------------------------------------------------------

void cnt::editor::apply()
{
    assert(world);

    world->set_param(key, str);
    is_changed = false;
}

void cnt::editor::show()
{
    assert(world);

    if ((count = world->get_param(key, str)) == 0)
        str = "";

    update();

    if (count > 1)
    {
        color[0] = 0xFF;
        color[1] = 0xFF;
        color[2] = 0x40;
    }
    else
    {
        color[0] = 0xFF;
        color[1] = 0xFF;
        color[2] = 0xFF;
    }

    is_enabled = (count > 0);
}

//-----------------------------------------------------------------------------

void cnt::bitmap::apply()
{
    assert(world);

    std::ostringstream sout;

    sout << bits;

    std::string val = sout.str();

    world->set_param(key, val);
    is_changed = false;
}

void cnt::bitmap::show()
{
    assert(world);

    std::string val;

    if ((count = world->get_param(key, val)) == 0)
        bits = 0;
    else
    {
        std::istringstream sin(val);
        sin >> bits;
    }

    if (count > 1)
    {
        color[0] = 0xFF;
        color[1] = 0xFF;
        color[2] = 0x40;
    }
    else
    {
        color[0] = 0xFF;
        color[1] = 0xFF;
        color[2] = 0xFF;
    }

    is_enabled = (count > 0);
}

//-----------------------------------------------------------------------------

void cnt::create_button::do_create(wrl::atom *atom)
{
    assert(world);
    assert(state);

    // Select the new entity and add a create operation for it.

    world->clear_selection();
    world->click_selection(atom);
    world->do_create();

    // Update the GUI to reflect the new entity state.

    state->show();
}

void cnt::new_ball_button::apply()
{
    do_create(new wrl::ball());
}

void cnt::new_hinge_button::apply()
{
    do_create(new wrl::hinge());
}

void cnt::new_hinge2_button::apply()
{
    do_create(new wrl::hinge2());
}

void cnt::new_slider_button::apply()
{
    do_create(new wrl::slider());
}

void cnt::new_amotor_button::apply()
{
    do_create(new wrl::amotor());
}

void cnt::new_universal_button::apply()
{
    do_create(new wrl::universal());
}

void cnt::new_box_button::apply()
{
    if (!name->value().empty())
        do_create(new wrl::box(name->value()));
}

void cnt::new_sphere_button::apply()
{
    if (!name->value().empty())
        do_create(new wrl::sphere(name->value()));
}

void cnt::new_d_light_button::apply()
{
    if (!name->value().empty())
        do_create(new wrl::d_light(name->value()));
}

void cnt::new_s_light_button::apply()
{
    if (!name->value().empty())
        do_create(new wrl::s_light(name->value()));
}

//-----------------------------------------------------------------------------

void cnt::init_button::apply()
{
    world->init();
    state->show();
}

void cnt::load_button::apply()
{
    if (!name->value().empty())
    {
        world->load(name->value());
        state->show();
    }
}

void cnt::save_all_button::apply()
{
    if (!name->value().empty())
    {
        world->save(name->value(), true);
        state->show();
    }
}

void cnt::save_sel_button::apply()
{
    if (!name->value().empty())
    {
        world->save(name->value(), false);
        state->show();
    }
}

//-----------------------------------------------------------------------------

void cnt::load_config_button::apply()
{
    if (!name->value().empty())
        ::host->reconfig(name->value());
}

//-----------------------------------------------------------------------------
// The About panel

cnt::about_panel::about_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    std::string text;

    try
    {
        text = std::string((const char *) ::data->load("ABOUT.md"));
        ::data->free("ABOUT.md");
    }
    catch (std::runtime_error& e)
    {
        text = "## ABOUT.md not found";
    }

    add((new gui::scroll)->
        add(new gui::pager(text)));
}

//-----------------------------------------------------------------------------
// The World control panel

cnt::world_panel::world_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    gui::selector *S = new gui::selector("world", ".xml");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::harray)->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new init_button    (W, w   ))->
                add(new save_sel_button(W, w, S))->
                add(new save_all_button(W, w, S))->
                add(new load_button    (W, w, S)))));
}

//-----------------------------------------------------------------------------
// The Solid control panel

cnt::solid_panel::solid_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    gui::selector *S = new gui::selector("solid", ".obj");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::harray)->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new new_box_button   (W, w, S))->
                add(new new_sphere_button(W, w, S)))));

    add(new gui::spacer);

    add((new gui::frame)->
        add((new gui::harray)->
            add((new gui::vgroup)->

                add(new title("Configure Solid", -1))->

                add((new gui::hgroup)->

                    add((new gui::varray)->

                        add(new label("Category"))->
                        add(new label("Collide")))->

                    add((new gui::varray)->

                        add(new bitmap(W, wrl::param::category))->
                        add(new bitmap(W, wrl::param::collide)))))->

            add((new gui::hgroup)->
                add((new gui::varray)->

                    add(new label("Density"))->
                    add(new label("Bounce"))->
                    add(new label("Friction")))->

                add((new gui::varray)->

                    add(new editor(W, wrl::param::density))->
                    add(new editor(W, wrl::param::bounce))->
                    add(new editor(W, wrl::param::mu)))->

                add((new gui::varray)->

                    add(new label("Soft ERP"))->
                    add(new label("Soft CFM"))->
                    add(new label("Follow")))->

                add((new gui::varray)->

                    add(new editor(W, wrl::param::soft_erp))->
                    add(new editor(W, wrl::param::soft_cfm))->
                    add(new editor(W, -1))))));
}

//-----------------------------------------------------------------------------
// The Joint control panel

cnt::joint_panel::joint_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    add((new gui::frame)->
        add((new gui::harray)->

            add(new title("New Joint"))->
            add(new new_ball_button     (W, w))->
            add(new new_hinge_button    (W, w))->
            add(new new_hinge2_button   (W, w))->
            add(new new_slider_button   (W, w))->
            add(new new_amotor_button   (W, w))->
            add(new new_universal_button(W, w))));

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

                add(new editor(W, dParamVel))->
                add(new editor(W, dParamFMax))->
                add(new editor(W, dParamCFM))->
                add(new editor(W, dParamBounce))->
                add(new editor(W, dParamLoStop))->
                add(new editor(W, dParamHiStop))->
                add(new editor(W, dParamStopERP))->
                add(new editor(W, dParamStopCFM))->
                add(new editor(W, dParamSuspensionERP))->
                add(new editor(W, dParamSuspensionCFM)))->

            add((new gui::varray)->
                add(new title("Axis 2"))->
                add(new editor(W, dParamVel2))->
                add(new editor(W, dParamFMax2))->
                add(new editor(W, dParamCFM2))->
                add(new editor(W, dParamBounce2))->
                add(new editor(W, dParamLoStop2))->
                add(new editor(W, dParamHiStop2))->
                add(new editor(W, dParamStopERP2))->
                add(new editor(W, dParamStopCFM2))->

                add(new gui::filler)->
                add(new gui::filler))->

            add((new gui::varray)->
                add(new title("Axis 3"))->

                add(new editor(W, dParamVel3))->
                add(new editor(W, dParamFMax3))->
                add(new editor(W, dParamCFM3))->
                add(new editor(W, dParamBounce3))->
                add(new editor(W, dParamLoStop3))->
                add(new editor(W, dParamHiStop3))->
                add(new editor(W, dParamStopERP3))->
                add(new editor(W, dParamStopCFM3))->

                add(new gui::filler)->
                add(new gui::filler))));
}

//-----------------------------------------------------------------------------
// The Light control panel

cnt::light_panel::light_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    gui::selector *S = new gui::selector("light", ".obj");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::harray)->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new gui::filler(true, false))->
                add(new new_d_light_button(W, w, S))->
                add(new new_s_light_button(W, w, S)))));

    add(new gui::spacer);

    add((new gui::frame)->
        add((new gui::harray)->
            add((new gui::hgroup)->

                add((new title("Configure Light")))->

                add((new gui::hgroup)->
                    add(new label("Brightness"))->
                    add(new editor(W, wrl::param::brightness)))->

                add((new gui::hgroup)->
                    add(new label("Attenuation"))->
                    add(new editor(W, wrl::param::attenuation)))->

                add((new gui::hgroup)->
                    add(new label("Cut-off Angle"))->
                    add(new editor(W, wrl::param::cutoff))))));
}

//-----------------------------------------------------------------------------
// The Config control panel

cnt::config_panel::config_panel(wrl::world *W, gui::widget *w) : gui::vgroup()
{
    const std::string a((const char *) glGetString(GL_VERSION));
    const std::string b((const char *) glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    const std::string s = "OpenGL " + a + " GLSL " + b;

    gui::selector *S = new gui::selector("host/common", ".xml");

    add((new gui::frame)->
        add((new gui::vgroup)->
            add(S)->
            add((new gui::hgroup)->
                add(new gui::filler(true, false))->
                add(new gui::string(s, gui::string::mono, 0, 0, 0, 0))->
                add(new load_config_button(S)))));
}

//-----------------------------------------------------------------------------
// The toplevel control panel

cnt::control::control(wrl::world *W, int w, int h)
{
    state = new gui::option;

    root = ((new gui::vgroup)->
            add((new gui::harray)->
                add(new title("Edworld", 0))->
                add(new panel_button("About",  state, 0))->
                add(new panel_button("World",  state, 1))->
                add(new panel_button("Solid",  state, 2))->
                add(new panel_button("Joint",  state, 3))->
                add(new panel_button("Light",  state, 4))->
                add(new panel_button("Config", state, 5))->
                add(new gui::spacer))->
            add(new gui::spacer)->
            add(state->
                add(new  about_panel(W, state))->
                add(new  world_panel(W, state))->
                add(new  solid_panel(W, state))->
                add(new  joint_panel(W, state))->
                add(new  light_panel(W, state))->
                add(new config_panel(W, state))));

    root->layup();
    root->laydn((w - root->get_w()) / 2,
                (h - root->get_h()) / 2,
                     root->get_w(),
                     root->get_h());
}

//-----------------------------------------------------------------------------
