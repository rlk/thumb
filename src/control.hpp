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

#include "world.hpp"
#include "font.hpp"
#include "gui.hpp"

//-----------------------------------------------------------------------------

namespace cnt
{
    //-------------------------------------------------------------------------
    // Titles and labels.

    class title : public gui::string
    {
    public:
        title(std::string s, int j=0) :
            gui::string(s, sans, j, 0xFF, 0xC0, 0x40) { }
    };

    class label : public gui::string
    {
    public:
        label(std::string s, int j=1) :
            gui::string(s, sans, j, 0xFF, 0xFF, 0xFF) { }
    };

    //-------------------------------------------------------------------------
    // Parameter editor.

    class editor : public gui::editor
    {
        wrl::world& world;
        int         count;
        int         key;

    public:

        editor(wrl::world& w, int k) :
            gui::editor(""), world(w), key(k) { }

        void apply();
        void show();
    };

    //-------------------------------------------------------------------------
    // Bitmap parameter editor.

    class bitmap : public gui::bitmap
    {
        wrl::world& world;
        int         count;
        int         key;

    public:

        bitmap(wrl::world& w, int k) :
            gui::bitmap(), world(w), key(k) { }

        void apply();
        void show();
    };

    //-------------------------------------------------------------------------
    // Entity creation buttons.

    class create_button : public gui::button
    {
    protected:

        wrl::world&  world;
        gui::widget *state;

        void do_create(wrl::atom *);

    public:

        create_button(wrl::world& w, gui::widget *s, std::string label) :
            gui::button(label), world(w), state(s) { }
    };

    //-------------------------------------------------------------------------
    // Joint creation buttons.

    class new_ball_button : public create_button
    {
    public:
        new_ball_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Ball") { }
        void apply();
    };

    class new_hinge_button : public create_button
    {
    public:
        new_hinge_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Hinge") { }
        void apply();
    };

    class new_hinge2_button : public create_button
    {
    public:
        new_hinge2_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Hinge2") { }
        void apply();
    };

    class new_slider_button : public create_button
    {
    public:
        new_slider_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Slider") { }
        void apply();
    };

    class new_amotor_button : public create_button
    {
    public:
        new_amotor_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "AMotor") { }
        void apply();
    };

    class new_universal_button : public create_button
    {
    public:
        new_universal_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Universal") { }
        void apply();
    };

    //-------------------------------------------------------------------------
    // Solid creation buttons.

    class new_box_button : public create_button
    {
        gui::widget *name;

    public:

        new_box_button(wrl::world& w, gui::widget *s, gui::widget *n) :
            create_button(w, s, "Box"), name(n) { }
        void apply();
    };

    class new_sphere_button : public create_button
    {
        gui::widget *name;

    public:

        new_sphere_button(wrl::world& w, gui::widget *s, gui::widget *n) :
            create_button(w, s, "Sphere"), name(n) { }
        void apply();
    };

    //-------------------------------------------------------------------------
    // Special creation buttons.

    class new_light_button : public create_button
    {
    public:

        new_light_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Light") { }
        void apply();
    };

    class new_camera_button : public create_button
    {
    public:

        new_camera_button(wrl::world& w, gui::widget *s) :
            create_button(w, s, "Camera") { }
        void apply();
    };

    //-------------------------------------------------------------------------
    // World file buttons.

    class init_button : public gui::button
    {
        wrl::world&  world;
        gui::widget *state;

    public:
        init_button(wrl::world& w, gui::widget *s) :
            gui::button("New"), world(w), state(s) { }

        void apply() { /*world.init();*/ state->show(); }
    };

    class load_button : public gui::button
    {
        wrl::world&  world;
        gui::widget *state;
        gui::widget *name;

    public:
        load_button(wrl::world& w, gui::widget *s, gui::widget *n) :
            gui::button("Load"), world(w), state(s), name(n) { }

        void apply() { /*world.load(name->value());*/ state->show(); }
    };

    class save_all_button : public gui::button
    {
        wrl::world&  world;
        gui::widget *state;
        gui::widget *name;

    public:
        save_all_button(wrl::world& w, gui::widget *s, gui::widget *n) :
            gui::button("Save All"), world(w), state(s), name(n) { }

        void apply() { /*world.save(name->value(), true);*/ state->show(); }
    };

    class save_sel_button : public gui::button
    {
        wrl::world&  world;
        gui::widget *state;
        gui::widget *name;

    public:
        save_sel_button(wrl::world& w, gui::widget *s, gui::widget *n) :
            gui::button("Save Selected"), world(w), state(s), name(n) { }

        void apply() { /*world.save(name->value(), false);*/ state->show(); }
    };

    //-------------------------------------------------------------------------
    // Panel selection button.

    class panel_button : public gui::button
    {
        gui::option *state;
        int          index;

    public:

        panel_button(std::string s, gui::option *w, int i) :
            button(s), state(w), index(i) { }

        void apply() { state->select(index); }
    };

    //-------------------------------------------------------------------------
    // Panels.

    class world_panel : public gui::vgroup
    {
    public:
        world_panel(wrl::world&, gui::widget *);
    };

    class solid_panel : public gui::vgroup
    {
    public:
        solid_panel(wrl::world&, gui::widget *);
    };

    class joint_panel : public gui::vgroup
    {
    public:
        joint_panel(wrl::world&, gui::widget *);
    };

    //-------------------------------------------------------------------------
    // Control panel dialog.

    class control : public gui::dialog
    {
    public:
        control(wrl::world&);
    };
}

//-----------------------------------------------------------------------------
