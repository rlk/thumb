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

#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <set>
#include <cstring>

//#include "batcher.hpp"

//-----------------------------------------------------------------------------

class constraint
{
protected:

    float M[16];
    float T[16];
/*
    ogl::element *rot[10];
    ogl::element *pos[10];
    ogl::batcher *bat;
    ogl::segment *seg;
*/
    int   mode;
    int   axis;
    int   grid;
    int   grid_a;
    float grid_d;

    float mouse_x;
    float mouse_y;
    float mouse_a;
    float mouse_d;

    void calc_rot(float&, float&, const float[3], const float[3]) const;
    void calc_pos(float&, float&, const float[3], const float[3]) const;

    void draw_rot(int) const;
    void draw_pos(int) const;

    void orient();

public:

    constraint();
   ~constraint();

    void set_mode(int);
    void set_axis(int);
    void set_grid(int);

    void set_transform(const float[16]);

    bool point(float[16], const float[3], const float[3]);
    void click(           const float[3], const float[3]);

    void draw() const;
};

//-----------------------------------------------------------------------------

#endif
