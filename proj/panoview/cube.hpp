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

#ifndef PANOVIEW_CUBE_HPP
#define PANOVIEW_CUBE_HPP

//-----------------------------------------------------------------------------

long long log2(long long n);

//-----------------------------------------------------------------------------

extern const double cube_v[8][3];
extern const int    cube_i[6][4];

long long  cube_size(long long);

long long  face_locate(long long, long long, long long, long long);
long long  face_child (long long, long long);
long long  face_index (long long);
long long  face_level (long long);
long long  face_parent(long long);

void face_neighbors(long long, long long&, long long&, long long&, long long&);

//-----------------------------------------------------------------------------

#endif
