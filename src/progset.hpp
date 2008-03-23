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

#ifndef PROGSET_HPP
#define PROGSET_HPP

#include "program.hpp"

//-----------------------------------------------------------------------------

namespace uni
{
    class progset
    {
        const ogl::program *plate;
        const ogl::program *polar;
        const ogl::program *strip;

    public:

        const static int prog_plate = 0;
        const static int prog_north = 1;
        const static int prog_south = 2;
        const static int prog_strip = 3;

        progset(std::string);
       ~progset();

        void bind(int=0) const;
        void free(int=0) const;

        void uniform(std::string, int)                            const;
        void uniform(std::string, double)                         const;
        void uniform(std::string, double, double)                 const;
        void uniform(std::string, double, double, double)         const;
        void uniform(std::string, double, double, double, double) const;
    };
}

//-----------------------------------------------------------------------------

#endif
