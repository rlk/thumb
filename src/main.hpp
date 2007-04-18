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

#ifndef MAIN_HPP
#define MAIN_HPP

#include "conf.hpp"
#include "lang.hpp"
#include "data.hpp"
#include "prog.hpp"
#include "font.hpp"
#include "glob.hpp"
#include "view.hpp"

//-----------------------------------------------------------------------------
// Global application state.

extern app::conf *conf;
extern app::lang *lang;
extern app::data *data;
extern app::prog *prog;
extern app::font *sans;
extern app::font *mono;
extern app::glob *glob;
extern app::view *view;

extern "C" {

    const void *load_dat(const char *, size_t *);
    void        free_dat(const char *);

    void        load_tex(const char *);
    void        free_tex(const char *);
}

//-----------------------------------------------------------------------------
// Expression queryable system state.

double get_time();
void   clr_time();

double get_key(int);
double get_btn(int);
double get_joy(int);

double get_trg(unsigned int);
void   set_trg(unsigned int);
void   clr_trg();

//-----------------------------------------------------------------------------

#endif
