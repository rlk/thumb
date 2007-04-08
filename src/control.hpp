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

#include "scene.hpp"
#include "joint.hpp"
#include "solid.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "main.hpp"
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
            gui::string(sans, s, j, 0xFF, 0xC0, 0x40) { }
    };

    class label : public gui::string
    {
    public:
        label(std::string s, int j=1) :
            gui::string(sans, s, j, 0xFF, 0xFF, 0xFF) { }
    };

    //-------------------------------------------------------------------------
    // Parameter editor.

    class editor : public gui::editor
    {
        ops::scene& scene;
        int         key;

    public:

        editor(ops::scene& s, int k) :
            gui::editor(""), scene(s), key(k) { }

        void apply();
        void show();
        void hide();
    };

    //-------------------------------------------------------------------------
    // Bitmap parameter editor.

    class bitmap : public gui::bitmap
    {
        ops::scene& scene;
        int         key;

    public:

        bitmap(ops::scene& s, int k) :
            gui::bitmap(), scene(s), key(k) { }

        void apply();
        void show();
        void hide();
    };

    //-------------------------------------------------------------------------
    // Entity creation buttons.

    class create_button : public gui::button
    {
    protected:

        ops::scene&  scene;
        gui::widget *state;

        void do_create(ent::entity *);

    public:

        create_button(ops::scene& s, gui::widget *w, std::string label) :
            gui::button(label), scene(s), state(w) { }
    };

    //-------------------------------------------------------------------------
    // Joint creation buttons.

    class new_ball_button : public create_button
    {
    public:
        new_ball_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "Ball") { }

        void apply() { do_create(new ent::ball()); }
    };

    class new_hinge_button : public create_button
    {
    public:
        new_hinge_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "Hinge") { }

        void apply() { do_create(new ent::hinge()); }
    };

    class new_hinge2_button : public create_button
    {
    public:
        new_hinge2_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "Hinge2") { }

        void apply() { do_create(new ent::hinge2()); }
    };

    class new_slider_button : public create_button
    {
    public:
        new_slider_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "Slider") { }

        void apply() { do_create(new ent::slider()); }
    };

    class new_amotor_button : public create_button
    {
    public:
        new_amotor_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "AMotor") { }

        void apply() { do_create(new ent::amotor()); }
    };

    class new_universal_button : public create_button
    {
    public:
        new_universal_button(ops::scene& s, gui::widget *w) :
            create_button(s, w, "Universal") { }

        void apply() { do_create(new ent::universal()); }
    };

    //-------------------------------------------------------------------------
    // Solid creation buttons.

    class new_box_button : public create_button
    {
        gui::widget *name;

    public:

        new_box_button(ops::scene&  s, gui::widget *w, gui::widget *n) :
            create_button(s, w, "Box"), name(n) { }

        void apply() {
            do_create(new ent::box(glob->load_geodata(name->value())));
        }
    };

    class new_sphere_button : public create_button
    {
        gui::widget *name;

    public:

        new_sphere_button(ops::scene&  s, gui::widget *w, gui::widget *n) :
            create_button(s, w, "Sphere"), name(n) { }

        void apply() {
            do_create(new ent::sphere(glob->load_geodata(name->value())));
        }
    };

    class new_capsule_button : public create_button
    {
        gui::widget *name;

    public:

        new_capsule_button(ops::scene&  s, gui::widget *w, gui::widget *n) :
            create_button(s, w, "Capsule"), name(n) { }

        void apply() {
            do_create(new ent::capsule(glob->load_geodata(name->value())));
        }
    };

    //-------------------------------------------------------------------------
    // Special creation buttons.

    class new_light_button : public create_button
    {
    public:

        new_light_button(ops::scene&  s, gui::widget *w) :
            create_button(s, w, "Light") { }

        void apply() {
            do_create(new ent::light());
        }
    };

    class new_camera_button : public create_button
    {
    public:

        new_camera_button(ops::scene&  s, gui::widget *w) :
            create_button(s, w, "Camera") { }

        void apply() {
            do_create(new ent::camera());
        }
    };

    //-------------------------------------------------------------------------
    // World file buttons.

    class init_button : public gui::button
    {
        ops::scene&  scene;
        gui::widget *state;

    public:
        init_button(ops::scene&s, gui::widget *w) :
            gui::button("New"), scene(s), state(w) { }

        void apply() { scene.init(); state->show(); }
    };

    class load_button : public gui::button
    {
        ops::scene&  scene;
        gui::widget *state;
        gui::widget *name;

    public:
        load_button(ops::scene&s, gui::widget *w, gui::widget *n) :
            gui::button("Load"), scene(s), state(w), name(n) { }

        void apply() { scene.load(name->value()); state->show(); }
    };

    class save_all_button : public gui::button
    {
        ops::scene&  scene;
        gui::widget *state;
        gui::widget *name;

    public:
        save_all_button(ops::scene&s, gui::widget *w, gui::widget *n) :
            gui::button("Save All"), scene(s), state(w), name(n) { }

        void apply() { scene.save(name->value(), true); state->show(); }
    };

    class save_sel_button : public gui::button
    {
        ops::scene&  scene;
        gui::widget *state;
        gui::widget *name;

    public:
        save_sel_button(ops::scene&s, gui::widget *w, gui::widget *n) :
            gui::button("Save Selected"), scene(s), state(w), name(n) { }

        void apply() { scene.save(name->value(), false); state->show(); }
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
        world_panel(ops::scene&, gui::widget *);
    };

    class solid_panel : public gui::vgroup
    {
    public:
        solid_panel(ops::scene&, gui::widget *);
    };

    class joint_panel : public gui::vgroup
    {
    public:
        joint_panel(ops::scene&, gui::widget *);
    };

    //-------------------------------------------------------------------------
    // Control panel dialog.

    class control : public gui::dialog
    {
    public:
        control(ops::scene&);
    };
}

//-----------------------------------------------------------------------------
