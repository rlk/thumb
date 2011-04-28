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
