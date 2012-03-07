//  Copyright (C) 2005-2012 Robert Kooima
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

#ifndef SPH_CRATER_HPP
#define SPH_CRATER_HPP

#include <string>
#include <vector>
#include <app-font.hpp>

//-----------------------------------------------------------------------------

class sph_crater
{
public:

    sph_crater(const std::string&);
   ~sph_crater();

    void draw(const double *, double, double);

private:

    struct crater
    {
        app::text *str;
        double d, p, l;

        crater(app::font& font, const std::string& name,
                                double d, double p, double l)
            : str(font.render(name)), d(d), p(p), l(l) { }

       ~crater() { delete str; }
    };

    GLint ring;

    std::vector<crater *> craters;
};

//-----------------------------------------------------------------------------

#endif
