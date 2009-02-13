//  Copyright (C) 2009 Robert Kooima
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

#ifndef OGL_RANGE_HPP
#define OGL_RANGE_HPP

//-----------------------------------------------------------------------------

namespace ogl
{
    class range
    {
        double n;
        double f;

    public:

        range(double, double);
        range();

        void merge(double);
        void merge(const range&);

        double get_n() const { return (valid() && n > 0.5    ) ? n : 0.5; }
        double get_f() const { return (valid() && f > get_n()) ? f : 5.0; }

        bool valid() const { return (n < f); }
    };
}

//-----------------------------------------------------------------------------

#endif
