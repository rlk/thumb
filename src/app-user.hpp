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

#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <mxml.h>

#include "ogl-program.hpp"
#include "app-serial.hpp"

//-----------------------------------------------------------------------------

namespace app
{
    class user
    {
    private:

        // View matrix and inverse cache

        double current_M[16];
        double current_I[16];
        double current_S[16];

        // Automatic demo file.

        app::serial file;

        app::node root;
        app::node prev;
        app::node curr;
        app::node next;
        app::node pred;

        app::node cycle_next(app::node);
        app::node cycle_prev(app::node);

        double interpolate(const char *, double);

        double tt;

    public:

        user();

        void get_point(double *, const double *,
                       double *, const double *) const;

        const double *get_M() const { return current_M; }
        const double *get_I() const { return current_I; }
        const double *get_S() const { return current_S; }

        // Interactive view controls.

        void turn(double, double, double, const double *);
        void turn(double, double, double);
        void move(double, double, double);
        void home();

        void tumble(const double *,
                    const double *);

        // Automatic view controls.

        bool dostep(double, double, double&, int&);
        void gonext();
        void goprev();
        void gohalf();
        void insert(double, int);
        void remove();
    };
}

//-----------------------------------------------------------------------------

extern app::user *user;

//-----------------------------------------------------------------------------

#endif
