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

#include <app-font.hpp>

#include "gui-loader.hpp"
#include "panoview.hpp"

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
    panoview    *P;
    gui::editor *E;
    
public:
    button_load(panoview *P, gui::editor *E)
        : gui::button("Load"), P(P), E(E) { }
    
    void apply()
    {
        if (!E->value().empty())
            P->load(E->value());
    }
};

class button_cancel : public gui::button
{
    panoview *P;
    
public:
    button_cancel(panoview *P)
        : gui::button("Cancel"), P(P) { }
    
    void apply()
    {
        P->cancel();
    }
};

//------------------------------------------------------------------------------

loader::loader(panoview *P, int w, int h)
{
    gui::editor *E = new gui::editor("");
    gui::finder *F = new gui::finder("pan", ".xml", E);

    root = ((new gui::frame)->
            add((new gui::vgroup)->
                add(label("PanoView \xE2\x80\x94 File Selection"))->

                add((new gui::hgroup)->
                    add(label("File:"))->
                    add(E))->
                    
                add(F)->
                
                add((new gui::hgroup)->
                    add(annot("Copyright \xC2\xA9 2011 Robert Kooima"))->
                    add(new gui::filler(true, false))->
                    add((new gui::harray)->
                        add(new button_load  (P, E))->
                        add(new button_cancel(P))))));

    root->layup();
    
    int ww = std::max(root->get_w(), 3 * w / 5);
    int hh = std::max(root->get_h(), 3 * h / 5);

    root->laydn((w - ww) / 2, (h - hh) / 2, ww, hh);
}

//------------------------------------------------------------------------------