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

#ifndef SPH_LABEL_HPP
#define SPH_LABEL_HPP

#include <string>
#include <vector>

#include "type.h"

//-----------------------------------------------------------------------------

class sph_label
{
public:

    sph_label(const void *, size_t,
              const void *, size_t);
   ~sph_label();

    void draw(const double *, double, double);

private:

    static const int strmax = 64;

    struct label
    {
        char  str[strmax];
        float lat;
        float lon;
        float dia;
    };

    void parse(const void *, size_t);

    GLint ring;
    GLint mark;

    font *label_font;
    line *label_line;

    std::vector<label> labels;
};

//-----------------------------------------------------------------------------

#endif
