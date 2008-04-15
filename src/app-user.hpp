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

        double default_M[16];
        double default_I[16];

        // Automatic demo state

        mxml_node_t *head;
        mxml_node_t *root;
        mxml_node_t *curr;

        bool dirty;

        double t0;
        double tt;
        double t1;

        double current_M0[16];
        double current_M1[16];

        double current_a0;
        double current_a1;
        double current_a;

        double current_t0;
        double current_t1;
        double current_t;

        void init();
        bool load();
        void save();

        void slerp(const double *,
                   const double *,
                   const double *, double);

    public:

        user();
       ~user();

        void get_point(double *, const double *,
                       double *, const double *) const;

        const double *get_M() const { return current_M; }
        const double *get_I() const { return current_I; }
        const double *get_S() const { return current_S; }

        void set_default();
        void get_default();

        // Interactive view controls.

        void turn(double, double, double, const double *);
        void turn(double, double, double);
        void move(double, double, double);
        void home();

        void tumble(const double *,
                    const double *);

        // Automatic view controls.

        bool dostep(double, const double *, double&, double&);
        void gocurr(double);
        void goinit(double);
        void gonext(double);
        void goprev(double);
        void insert(double, double);
        void remove();
    };
}

//-----------------------------------------------------------------------------

extern app::user *user;

//-----------------------------------------------------------------------------

#endif
