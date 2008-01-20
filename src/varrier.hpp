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

#ifndef VARRIER_HPP
#define VARRIER_HPP

#include "serial.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class varrier
    {
        app::node node;

        double pitch;
        double angle;
        double thick;
        double shift;
        double cycle;

    public:

        varrier(app::node);
        
        // Calibration input handler

        bool input_keybd(int, int, int, bool);

        void draw() const;
    };
}

//-----------------------------------------------------------------------------

#endif
