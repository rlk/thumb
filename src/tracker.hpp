//  Copyright (C) 2007 Robert Kooima
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

#ifndef TRACKER_HPP
#define TRACKER_HPP

//-----------------------------------------------------------------------------

bool tracker_init(int, int);
void tracker_fini(void);

bool tracker_status(void);

//-----------------------------------------------------------------------------

bool tracker_sensor(int, double[3], double[4]);
bool tracker_values(int, double&);
bool tracker_button(int, bool&);

//-----------------------------------------------------------------------------

#endif
