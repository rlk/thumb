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

//------------------------------------------------------------------------------

loader::loader(int w, int h)
{
    gui::editor *E = new gui::editor("");
    gui::finder *F = new gui::finder("pan", ".xml", E);

    root = ((new gui::vgroup)->
                add((new gui::hgroup)->
                    add(new gui::string("Panorama", 0, 0, 0xFF, 0xC0, 0x40))->
                    add(E))->
                add(F));

    root->layup();
    root->laydn(w / 4, h / 4, w / 2, h / 2);
}

//------------------------------------------------------------------------------
