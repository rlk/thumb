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

#include "cube.hpp"

//-----------------------------------------------------------------------------

const double cube_v[8][3] = {
    {  1,  1,  1 },
    { -1,  1,  1 },
    {  1, -1,  1 },
    { -1, -1,  1 },
    {  1,  1, -1 },
    { -1,  1, -1 },
    {  1, -1, -1 },
    { -1, -1, -1 },
};

const int cube_i[6][4] = {
    { 0, 4, 2, 6 },
    { 5, 1, 7, 3 },
    { 5, 4, 1, 0 },
    { 3, 2, 7, 6 },
    { 1, 0, 3, 2 },
    { 4, 5, 6, 7 },
};

//-----------------------------------------------------------------------------
