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

#ifndef RECT_HPP
#define RECT_HPP

//-----------------------------------------------------------------------------

struct rect
{
    int x;
    int y;
    int w;
    int h;

    rect(int a, int b, int c, int d) : x(a), y(b), w(c), h(d) { }

    bool test(int X, int Y) const { return (x <= X && X <= x + w &&
                                            y <= Y && Y <= y + h); }
    int L() const { return x;     }
    int R() const { return x + w; }
    int B() const { return y;     }
    int T() const { return y + h; }
};

typedef std::vector<rect> rect_v;

//-----------------------------------------------------------------------------

#endif
