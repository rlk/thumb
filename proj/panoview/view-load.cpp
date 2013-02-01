#include <app-font.hpp>

#include "view-load.hpp"
#include "view-app.hpp"

//------------------------------------------------------------------------------

static gui::widget *label(const std::string& text)
{
    return new gui::string(text, 0, 0, 0xFF, 0xC0, 0x40);
}

static gui::widget *annot(const std::string& text)
{
    return new gui::string(text, 0, 0, 0x20, 0x10, 0x00);
}

//------------------------------------------------------------------------------

class button_load_data : public gui::button
{
    view_app    *V;
    gui::editor *E;

public:
    button_load_data(view_app *V, gui::editor *E)
        : gui::button("Load Data"), V(V), E(E) { }

    void apply()
    {
        if (!E->value().empty())
            V->load_file(E->value());
    }
};

class button_load_path : public gui::button
{
    view_app    *V;
    gui::editor *E;

public:
    button_load_path(view_app *V, gui::editor *E)
        : gui::button("Load Path"), V(V), E(E) { }

    void apply()
    {
        if (!E->value().empty())
            V->load_path(E->value());
    }
};

class button_cancel : public gui::button
{
    view_app *V;

public:
    button_cancel(view_app *V)
        : gui::button("Cancel"), V(V) { }

    void apply()
    {
        V->cancel();
    }
};

//------------------------------------------------------------------------------

#define SP "                                                                   "

view_load::view_load(view_app *V, int w, int h)
{
    gui::editor *E = new gui::editor("");
    gui::finder *F = new gui::finder("scm", ".xml", E);

    if (char *name = getenv("SCMINIT"))
        E->value(name);

    load_data = new button_load_data(V, E);
    load_path = new button_load_path(V, E);

    root = ((new gui::frame)->
            add((new gui::vgroup)->
                add(label("SCM Viewer \xE2\x80\x94 File Selection"))->

                add((new gui::hgroup)->
                    add(label("File:"))->
                    add(E))->

                add(F)->

                add((new gui::hgroup)->
                    add(annot("Copyright \xC2\xA9 2011-13 Robert Kooima"))->
                    add(new gui::filler(true, false))->
                    add((new gui::harray)->
                        add(load_path)->
                        add(load_data)->
                        add(new button_cancel(V))))));

    root->layup();

    int ww = std::max(root->get_w(), 3 * w / 5);
    int hh = std::max(root->get_h(), 3 * h / 5);

    root->laydn((w - ww) / 2, (h - hh) / 2, ww, hh);
}

void view_load::reload()
{
    load_data->apply();
}

//------------------------------------------------------------------------------

