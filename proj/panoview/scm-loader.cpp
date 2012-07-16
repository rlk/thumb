#include <app-font.hpp>

#include "view-load.hpp"
#include "scm-viewer.hpp"

//------------------------------------------------------------------------------

static gui::widget *label(const std::string& text)
{
    return new gui::string(text, 0, 0, 0xFF, 0xC0, 0x40);
}

static gui::widget *annot(const std::string& text)
{
    return new gui::string(text, 0, 0, 0x00, 0x00, 0x00);
}

//------------------------------------------------------------------------------

class button_load : public gui::button
{
    view_app  *V;
    gui::editor *E;

public:
    button_load(view_app *V, gui::editor *E)
        : gui::button("Load"), V(V), E(E) { }

    void apply()
    {
        if (!E->value().empty())
            V->load(E->value());
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

    status = new gui::string(SP, 0, 0, 0xFF, 0xC0, 0x40);

    root = ((new gui::frame)->
            add((new gui::vgroup)->
                add(label("SCM Viewer \xE2\x80\x94 File Selection"))->

                add((new gui::hgroup)->
                    add(label("File:"))->
                    add(E))->

                add(F)->

                add((new gui::hgroup)->
                    add(annot("Copyright \xC2\xA9 2011 Robert Kooima"))->
                    add(new gui::filler(true, false))->
                    add(status)->
                    add(new gui::filler(true, false))->
                    add((new gui::harray)->
                        add(new button_load  (V, E))->
                        add(new button_cancel(V))))));

    root->layup();

    int ww = std::max(root->get_w(), 3 * w / 5);
    int hh = std::max(root->get_h(), 3 * h / 5);

    root->laydn((w - ww) / 2, (h - hh) / 2, ww, hh);
}

void view_load::set_status(int c)
{
    char buf[256];

    sprintf(buf, "Cache size: %d KB", c / 1024);
    status->value(buf);
}

//------------------------------------------------------------------------------

